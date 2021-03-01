//
// aegis - project change supervisor
// Copyright (C) 1994-1999, 2001-2009, 2012 Peter Miller
// Copyright (C) 2007, 2008 Walter Franzini
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/libintl.h>

#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/trace.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/change.h>
#include <libaegis/commit.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/pstate.fmtgen.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aechown.h>


static void
change_owner_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "Usage: %s -Change_Owner [ <option>... ]\n", progname);
    fprintf
    (
        stderr,
        "       %s -Change_Owner -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Change_Owner -Help\n", progname);
    quit(1);
}


static void
change_owner_help(void)
{
    help("aechown", change_owner_usage);
}


static void
change_owner_list(void)
{
    trace(("change_owner_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(change_owner_usage);
    list_changes_in_state_mask(cid, 1 << cstate_state_being_developed);
    trace(("}\n"));
}


static void
change_owner_main(void)
{
    sub_context_ty  *scp;
    user_ty::pointer up1;
    user_ty::pointer up2;
    cstate_ty       *cstate_data;
    cstate_history_ty *history_data;
    string_ty       *new_developer;
    string_ty       *devdir;
    string_ty       *old_dd;
    size_t          j;
    pconf_ty        *pconf_data;

    trace(("change_owner_main()\n{\n"));
    arglex();
    change_identifier cid;
    new_developer = 0;
    devdir = 0;
    string_ty *reason = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(change_owner_usage);
            continue;

        case arglex_token_keep:
        case arglex_token_interactive:
        case arglex_token_keep_not:
            user_ty::delete_file_argument(change_owner_usage);
            break;

        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_trunk:
            cid.command_line_parse(change_owner_usage);
            continue;

        case arglex_token_directory:
            if (devdir)
                duplicate_option(change_owner_usage);
            if (arglex() != arglex_token_string)
                option_needs_dir(arglex_token_directory, change_owner_usage);
            devdir = str_from_c(arglex_value.alv_string);
            break;

        case arglex_token_user:
            if (arglex() != arglex_token_string)
                option_needs_name(arglex_token_user, change_owner_usage);
            // fall through...

        case arglex_token_string:
            if (new_developer)
                duplicate_option_by_name(arglex_token_user, change_owner_usage);
            new_developer = str_from_c(arglex_value.alv_string);
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(change_owner_usage);
            break;

        case arglex_token_reason:
            if (reason)
                duplicate_option(change_owner_usage);
            switch (arglex())
            {
            default:
                option_needs_string(arglex_token_reason, change_owner_usage);
                // NOTREACHED

            case arglex_token_string:
            case arglex_token_number:
                reason = str_from_c(arglex_value.alv_string);
                break;
            }
            break;
        }
        arglex();
    }
    if (!cid.set())
        fatal_intl(0, i18n("no change number"));
    if (!new_developer)
        fatal_intl(0, i18n("no user name"));

    //
    // it is an error if the named user is not a developer
    //
    if (!project_developer_query(cid.get_pp(), nstring(new_developer)))
    {
        scp = sub_context_new();
        sub_var_set_string(scp, "Target", new_developer);
        project_fatal(cid.get_pp(), scp, i18n("$target not developer"));
        // NOTREACHED
        sub_context_delete(scp);
    }

    //
    // It is an error if the change is not in the 'being developed' state
    //
    if (!cid.get_cp()->is_being_developed())
        change_fatal(cid.get_cp(), 0, i18n("bad chown state"));

    //
    // Get details of the two users involved.
    //
    up1 = user_ty::create(nstring(cid.get_cp()->developer_name()));
    trace(("up1 = %08lX\n", (long)up1.get()));
    up2 = user_ty::create(nstring(new_developer));
    trace(("up2 = %08lX\n", (long)up2.get()));

    //
    // It is an error if the executing user is not a project
    // administrator.  However, developers are allowed to chown it
    // to themselves, because this is a common way of moving the
    // development directory of a change.
    //
    if (str_equal(cid.get_cp()->developer_name(), new_developer))
        change_verbose(cid.get_cp(), 0, i18n("warning: no chown"));
    else if (!project_administrator_query(cid.get_pp(), cid.get_up()->name()))
        project_fatal(cid.get_pp(), 0, i18n("not an administrator"));

    //
    // Take an advisory write lock on the appropriate row of the change
    // table.  Take an advisory write lock on the appropriate row of the
    // user table.  Block until can get both simultaneously.
    //
    cid.get_pp()->pstate_lock_prepare();
    change_cstate_lock_prepare(cid.get_cp());
    up1->ustate_lock_prepare();
    up2->ustate_lock_prepare();
    lock_take();
    cstate_data = cid.get_cp()->cstate_get();

    //
    // These could have changed, check again:
    // It is an error if the change is not in the being developed state.
    // It is an error if the change is already being developed by the
    // named user.
    //
    if (nstring(cid.get_cp()->developer_name()) != up1->name())
        change_fatal(cid.get_cp(), 0, i18n("sync error, try again"));
    if (cid.get_cp()->is_a_branch())
        change_fatal(cid.get_cp(), 0, i18n("no branch chown"));

    //
    // add to history for state change
    //
    string_ty *reason2 =
        str_format
        (
            "Forced by administrator \"%s\".",
            cid.get_up()->name().c_str()
        );
    if (reason)
        reason = str_format("%s\n%s", reason->str_text, reason2->str_text);
    else
        reason = reason2;
    history_data = change_history_new(cid.get_cp(), up1);
    history_data->what = cstate_history_what_develop_begin_undo;
    history_data->why = str_copy(reason);
    history_data = change_history_new(cid.get_cp(), up2);
    history_data->what = cstate_history_what_develop_begin;
    history_data->why = reason;

    //
    // Clear the build-time field.
    // Clear the test-time field.
    // Clear the test-baseline-time field.
    // Clear the src field.
    //
    change_build_times_clear(cid.get_cp());

    //
    // Remove the change from the list of assigned changes in the user
    // change table (in the user row).
    //
    up1->own_remove(cid.get_pp(), cid.get_change_number());
    up2->own_add(cid.get_pp(), cid.get_change_number());

    //
    // Create the change directory.
    //
    if (!devdir)
    {
        scp = sub_context_new();
        devdir = change_development_directory_template(cid.get_cp(), up2);
        sub_var_set_string(scp, "File_Name", devdir);
        change_verbose
        (
            cid.get_cp(),
            scp,
            i18n("development directory \"$filename\"")
        );
        sub_context_delete(scp);
    }
    assert(cstate_data->development_directory);
    old_dd = cstate_data->development_directory;
    cstate_data->development_directory = devdir;

    //
    // Create the development directory.
    //
    up2->become_begin();
    os_mkdir(devdir, 02755);
    undo_rmdir_errok(devdir);
    up2->become_end();
    cid.get_cp()->run_develop_begin_early_command(up2);

    //
    // Make sure fstate read in so that it does not do so during
    // the loop (otherwise multiple user permissions set).
    //
    change_file_nth(cid.get_cp(), 0, view_path_first);

    //
    // copy change files across
    //      (even the removed files)
    //
    change_verbose(cid.get_cp(), 0, i18n("copy change source files"));
    up2->become_begin();
    for (j = 0;; ++j)
    {
        string_ty       *s1;
        fstate_src_ty   *src_data;

        //
        // copy the file across
        //
        src_data = change_file_nth(cid.get_cp(), j, view_path_first);
        if (!src_data)
            break;
        s1 = os_path_join(old_dd, src_data->file_name);
        if (os_exists(s1))
        {
            string_ty       *s2;

            s2 = os_path_join(devdir, src_data->file_name);
            os_mkdir_between(devdir, src_data->file_name, 02755);
            copy_whole_file(s1, s2, 0);
            str_free(s2);
        }
        str_free(s1);

        //
        // clear the file time stamps
        //
        if (src_data->file_fp)
        {
            fingerprint_type.free(src_data->file_fp);
            src_data->file_fp = 0;
        }
        if (src_data->diff_file_fp)
        {
            fingerprint_type.free(src_data->diff_file_fp);
            src_data->diff_file_fp = 0;
        }
        if (src_data->architecture_times)
        {
            fstate_src_architecture_times_list_type.free
            (
                src_data->architecture_times
            );
            src_data->architecture_times = 0;
        }
    }
    up2->become_end();

    //
    // remove the old development directory
    //
    if (cid.get_up()->delete_file_query(nstring(old_dd), true, true))
    {
        change_verbose
        (
            cid.get_cp(),
            0,
            i18n("remove old development directory")
        );
        user_ty::become scoped(up1);
        commit_rmdir_tree_errok(old_dd);
    }
    str_free(old_dd);

    //
    // Write the change table row.
    // Write the user table rows.
    // Release advisory locks.
    //
    cid.get_cp()->cstate_write();
    cid.get_pp()->pstate_write();
    up1->ustate_write();
    up2->ustate_write();
    commit();
    lock_release();

    //
    // run the notification commands
    //
    change_run_develop_begin_undo_command(cid.get_cp(), up1);
    cid.get_cp()->run_develop_begin_command(up2);
    cid.get_cp()->run_forced_develop_begin_notify_command(cid.get_up());

    //
    // if symlinks are being used to pander to dumb DMT,
    // and they are not removed after each build,
    // create them now, rather than waiting for the first build.
    // This will present a more uniform interface to the developer.
    //
    pconf_data = change_pconf_get(cid.get_cp(), 0);
    assert(pconf_data->development_directory_style);
    if (!pconf_data->development_directory_style->during_build_only)
    {
        work_area_style_ty style = *pconf_data->development_directory_style;
        change_create_symlinks_to_baseline(cid.get_cp(), up2, style);
    }

    //
    // verbose success message
    //
    scp = sub_context_new();
    sub_var_set_string(scp, "ORiginal", up1->name());
    sub_var_optional(scp, "ORiginal");
    sub_var_set_string(scp, "Target", up2->name());
    sub_var_optional(scp, "Target");
    change_verbose(cid.get_cp(), scp, i18n("chown complete"));
    sub_context_delete(scp);

    trace(("}\n"));
}


void
change_owner(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, change_owner_help, 0 },
        { arglex_token_list, change_owner_list, 0 },
    };

    trace(("change_owner()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), change_owner_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
