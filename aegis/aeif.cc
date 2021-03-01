//
//      aegis - project change supervisor
//      Copyright (C) 1991-2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/error.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/trace.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/develop_direct/read_write.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/project/history.h>
#include <libaegis/rss.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aeif.h>


static void
integrate_fail_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -Integrate_FAIL -File <reason-file> [ <option>... ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -Integrate_FAIL -REAson '<text>' [ <option>... ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -Integrate_FAIL -Edit [ <option>... ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -Integrate_FAIL -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Integrate_FAIL -Help\n", progname);
    quit(1);
}


static void
integrate_fail_help(void)
{
    help("aeifail", integrate_fail_usage);
}


static void
integrate_fail_list(void)
{
    trace(("integrate_fail_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(integrate_fail_usage);
    list_changes_in_state_mask(cid, 1 << cstate_state_being_integrated);
    trace(("}\n"));
}


static void
check_directory(change::pointer cp)
{
    string_ty       *dir;

    dir = change_integration_directory_get(cp, 1);
    os_become_orig();
    if (os_below_dir(dir, os_curdir()))
        change_fatal(cp, 0, i18n("leave int dir"));
    os_become_undo();
}


static void
check_permissions(change::pointer cp, user_ty::pointer up)
{
    cstate_ty       *cstate_data;

    cstate_data = cp->cstate_get();

    //
    // it is an error if the change is not in the 'being_integrated' state.
    //
    if (cstate_data->state != cstate_state_being_integrated)
        change_fatal(cp, 0, i18n("bad if state"));
    if (nstring(change_integrator_name(cp)) != up->name())
        change_fatal(cp, 0, i18n("not integrator"));
}


static void
integrate_fail_main(void)
{
    sub_context_ty  *scp;
    cstate_ty       *cstate_data;
    cstate_history_ty *history_data;
    const char      *reason =       0;
    string_ty       *dir;
    int             j;
    string_ty       *project_name;
    project_ty      *pp;
    long            change_number;
    change::pointer cp;
    user_ty::pointer up;
    user_ty::pointer devup;
    edit_ty         edit;

    trace(("integrate_fail_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    edit = edit_not_set;
    nstring comment;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(integrate_fail_usage);
            continue;

        case arglex_token_string:
            scp = sub_context_new();
            sub_var_set_charstar
            (
                scp,
                "Name",
                arglex_token_name(arglex_token_file)
            );
            error_intl(scp, i18n("warning: use $name option"));
            sub_context_delete(scp);
            if (!comment.empty())
                fatal_too_many_files();
            goto read_reason_file;

        case arglex_token_file:
            if (!comment.empty())
                duplicate_option(integrate_fail_usage);
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_file, integrate_fail_usage);
                // NOTREACHED

            case arglex_token_string:
                read_reason_file:
                os_become_orig();
                comment = read_whole_file(nstring(arglex_value.alv_string));
                os_become_undo();
                break;

            case arglex_token_stdio:
                os_become_orig();
                comment = read_whole_file(nstring());
                os_become_undo();
                break;
            }
            assert(!comment.empty());
            break;

        case arglex_token_reason:
            if (reason)
                duplicate_option(integrate_fail_usage);
            if (arglex() != arglex_token_string)
                option_needs_string(arglex_token_reason, integrate_fail_usage);
            reason = arglex_value.alv_string;
            break;

        case arglex_token_change:
            arglex();
            // fall through...

        case arglex_token_number:
            arglex_parse_change
            (
                &project_name,
                &change_number,
                integrate_fail_usage
            );
            continue;

        case arglex_token_project:
            arglex();
            arglex_parse_project(&project_name, integrate_fail_usage);
            continue;

        case arglex_token_edit:
            if (edit == edit_foreground)
                duplicate_option(integrate_fail_usage);
            if (edit != edit_not_set)
            {
                too_many_edits:
                mutually_exclusive_options
                (
                    arglex_token_edit,
                    arglex_token_edit_bg,
                    integrate_fail_usage
                );
            }
            edit = edit_foreground;
            break;

        case arglex_token_edit_bg:
            if (edit == edit_background)
                duplicate_option(integrate_fail_usage);
            if (edit != edit_not_set)
                goto too_many_edits;
            edit = edit_background;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(integrate_fail_usage);
            break;
        }
        arglex();
    }
    if (!comment.empty() && reason)
    {
        mutually_exclusive_options
        (
            arglex_token_file,
            arglex_token_reason,
            integrate_fail_usage
        );
    }
    if (edit != edit_not_set && (!comment.empty() || reason))
    {
        mutually_exclusive_options
        (
            (
                edit == edit_foreground
            ?
                arglex_token_edit
            :
                arglex_token_edit_bg
            ),
            (!comment.empty() ? arglex_token_file : arglex_token_reason),
            integrate_fail_usage
        );
    }
    if (edit == edit_not_set && comment.empty() && !reason)
    {
        sub_context_ty sc;
        sc.var_set_charstar
        (
            "Name1",
            arglex_token_name(arglex_token_file)
        );
        sc.var_set_charstar
        (
            "Name2",
            arglex_token_name(arglex_token_edit)
        );
        sc.error_intl(i18n("warning: no $name1, assuming $name2"));
        edit = edit_foreground;
    }
    if (reason)
        comment = nstring(reason);

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // locate change data
    //
    if (!change_number)
        change_number = up->default_change(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // create the comments, if required
    //
    if (edit != edit_not_set)
    {
        //
        // make sure they are not in the integration directory,
        // to avoid a wasted edit
        //
        check_directory(cp);

        //
        // make sure they are allowed to first,
        // to avoid a wasted edit
        //
        check_permissions(cp, up);
        comment = nstring(os_edit_new(edit));
    }

    //
    // lock the change for writing
    //
    pp->pstate_lock_prepare();
    change_cstate_lock_prepare(cp);
    lock_prepare_ustate_all(0, 0); // we don't know which users until later
    lock_take();
    cstate_data = cp->cstate_get();

    //
    // make sure they are allowed to
    // (even if edited, could have changed during edit)
    //
    check_permissions(cp, up);

    //
    // Change the state.
    // Add to the change's history.
    //
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_integrate_fail;
    if (!comment.empty())
        history_data->why = str_copy(comment.get_ref());
    change_build_times_clear(cp);
    cstate_data->delta_number = 0;
    if (cstate_data->delta_uuid)
    {
        str_free(cstate_data->delta_uuid);
        cstate_data->delta_uuid = 0;
    }
    dir = str_copy(change_integration_directory_get(cp, 1));
    change_integration_directory_clear(cp);
    cstate_data->state = cstate_state_being_developed;
    cstate_data->minimum_integration = false;

    //
    // Complain if they are in the integration directory,
    // because the rmdir at the end can't then run to completion.
    //
    check_directory(cp);

    //
    // note that the project has no current integration
    //
    project_current_integration_set(pp, 0);

    //
    // Remove it from the integrator's change list, and
    // add it back into the developer's change list.
    //
    up->own_remove(pp, change_number);
    devup = user_ty::create(nstring(change_developer_name(cp)));
    devup->own_add(pp, change_number);

    //
    // go through the files in the change and unlock them
    // in the baseline
    //
    for (j = 0;; ++j)
    {
        fstate_src_ty   *c_src_data;
        fstate_src_ty   *p_src_data;

        c_src_data = change_file_nth(cp, j, view_path_first);
        if (!c_src_data)
            break;
        p_src_data =
            project_file_find(pp, c_src_data->file_name, view_path_none);
        if (!p_src_data)
        {
            // This is actualy a bug.
            continue;
        }
        p_src_data->locked_by = 0;

        //
        // Remove the file if it is about_to_be_created
        // by the change we are rescinding.
        //
        if
        (
            p_src_data->about_to_be_created_by
        ||
            p_src_data->about_to_be_copied_by
        )
        {
            assert(!p_src_data->about_to_be_created_by ||
                p_src_data->about_to_be_created_by==change_number);
            assert(!p_src_data->about_to_be_copied_by ||
                p_src_data->about_to_be_copied_by==change_number);
            if (p_src_data->deleted_by)
            {
                assert(!p_src_data->about_to_be_copied_by);
                p_src_data->about_to_be_created_by = 0;
            }
            else
                project_file_remove(pp, c_src_data->file_name);
        }

        //
        // remove the integrate difference time
        //
        if (c_src_data->idiff_file_fp)
        {
            fingerprint_type.free(c_src_data->idiff_file_fp);
            c_src_data->idiff_file_fp = 0;
        }

        //
        // remove file test times
        //
        if (c_src_data->architecture_times)
        {
            fstate_src_architecture_times_list_type.free
            (
                c_src_data->architecture_times
            );
            c_src_data->architecture_times = 0;
        }
    }

    //
    // remove the integration directory
    //
    change_verbose(cp, 0, i18n("rm int dir"));
    project_become(pp);
    commit_rmdir_tree_bg(dir);
    project_become_undo(pp);

    //
    // Make the development directory writable again.
    //
    // This is actually conditional upon project_protect_development_
    // directory_get(pp) but the test is inside the change_
    // development_directory_chmod_read_write(cp) function, because it
    // also makes sure the source files are readable and writable by the
    // developer.
    //
    change_development_directory_chmod_read_write(cp);

    //
    // write out the data and release the locks
    //
    change_cstate_write(cp);
    up->ustate_write();
    devup->ustate_write();
    pp->pstate_write();
    str_free(dir);
    commit();
    lock_release();

    //
    // run the notify command
    //
    cp->run_integrate_fail_notify_command();

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(pp, cp);

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("integrate fail complete"));
    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}


void
integrate_fail(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, integrate_fail_help, 0 },
        { arglex_token_list, integrate_fail_list, 0 },
    };

    trace(("integrate_fail()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), integrate_fail_main);
    trace(("}\n"));
}
