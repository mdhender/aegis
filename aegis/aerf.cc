//
// aegis - project change supervisor
// Copyright (C) 1991-1999, 2001-2008, 2011, 2012 Peter Miller
// Copyright (C) 2008 Walter Franzini
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
#include <common/ac/stdlib.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/mem.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/trace.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/develop_direct/read_write.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/pattr.fmtgen.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/rss.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aerf.h>


static void
review_fail_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -Review_FAIL -File <reason-file> [ <option>... ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -Review_FAIL -REAson '<text>' [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Review_FAIL -Edit [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Review_FAIL -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Review_FAIL -Help\n", progname);
    quit(1);
}


static void
review_fail_help(void)
{
    help("aerfail", review_fail_usage);
}


static void
review_fail_list(void)
{
    trace(("review_fail_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(review_fail_usage);
    list_changes_in_state_mask(cid, 1 << cstate_state_being_reviewed);
    trace(("}\n"));
}


static void
check_permissions(change::pointer cp, user_ty::pointer up)
{
    cstate_ty       *cstate_data;
    cstate_history_ty *hp;

    cstate_data = cp->cstate_get();

    //
    // it is an error if the change is not in the 'being_reviewed' state.
    //
    if (cstate_data->state != cstate_state_being_reviewed)
        change_fatal(cp, 0, i18n("bad rf state"));
    assert(cstate_data->history->length >= 3);
    hp = cstate_data->history->list[cstate_data->history->length - 1];
    if (hp->what == cstate_history_what_review_begin)
    {
        if (nstring(cp->reviewer_name()) != up->name())
            change_fatal(cp, 0, i18n("not reviewer"));
    }
    else
    {
        project      *pp;

        assert(hp->what == cstate_history_what_develop_end);
        pp = cp->pp;
        if (!project_reviewer_query(pp, up->name()))
            project_fatal(pp, 0, i18n("not a reviewer"));
        if
        (
            !project_developer_may_review_get(pp)
        &&
            nstring(cp->developer_name()) == up->name()
        )
        {
            change_fatal(cp, 0, i18n("developer may not review"));
        }
    }
}


static void
review_fail_main(void)
{
    trace(("review_fail_main()\n{\n"));
    arglex();
    change_identifier cid;
    edit_ty edit = edit_not_set;
    nstring comment;
    nstring reason;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(review_fail_usage);
            continue;

        case arglex_token_string:
            {
                sub_context_ty sc;
                sc.var_set_charstar
                (
                    "Name",
                    arglex_token_name(arglex_token_file)
                );
                error_intl(&sc, i18n("warning: use $name option"));
                if (!comment.empty())
                    fatal_too_many_files();
            }
            goto read_input_file;

        case arglex_token_file:
            if (!comment.empty())
                duplicate_option(review_fail_usage);
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_file, review_fail_usage);
                // NOTREACHED

            case arglex_token_string:
                read_input_file:
                os_become_orig();
                comment = read_whole_file(arglex_value.alv_string);
                os_become_undo();
                break;

            case arglex_token_stdio:
                os_become_orig();
                comment = read_whole_file((char *)0);
                os_become_undo();
                break;
            }
            break;

        case arglex_token_reason:
            if (!reason.empty())
                duplicate_option(review_fail_usage);
            if (arglex() != arglex_token_string)
                option_needs_string(arglex_token_reason, review_fail_usage);
            reason = nstring(arglex_value.alv_string);
            break;

        case arglex_token_change:
        case arglex_token_number:
        case arglex_token_project:
            cid.command_line_parse(review_fail_usage);
            continue;

        case arglex_token_edit:
            if (edit == edit_foreground)
                duplicate_option(review_fail_usage);
            if (edit != edit_not_set)
            {
                too_many_edits:
                mutually_exclusive_options
                (
                    arglex_token_edit,
                    arglex_token_edit_bg,
                    review_fail_usage
                );
            }
            edit = edit_foreground;
            break;

        case arglex_token_edit_bg:
            if (edit == edit_background)
                duplicate_option(review_fail_usage);
            if (edit != edit_not_set)
                goto too_many_edits;
            edit = edit_background;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(review_fail_usage);
            break;
        }
        arglex();
    }
    cid.command_line_check(review_fail_usage);
    if (!comment.empty() && !reason.empty())
    {
        mutually_exclusive_options
        (
            arglex_token_file,
            arglex_token_reason,
            review_fail_usage
        );
    }
    if (edit != edit_not_set && (!comment.empty() || !reason.empty()))
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
            (comment.empty() ? arglex_token_reason : arglex_token_file),
            review_fail_usage
        );
    }
    if (edit == edit_not_set && comment.empty() && reason.empty())
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
        error_intl(&sc, i18n("warning: no $name1, assuming $name2"));
        edit = edit_foreground;
    }
    if (reason)
        comment = reason;

    project *pp = cid.get_pp();
    user_ty::pointer up = cid.get_up();
    change::pointer cp = cid.get_cp();

    //
    // create the comment, if necessary
    // check permissions first
    //
    if (edit != edit_not_set)
    {
        check_permissions(cp, up);
        comment = nstring(os_edit_new(edit));
    }

    //
    // lock the change for writing
    //
    pp->pstate_lock_prepare();
    change_cstate_lock_prepare(cp);
    lock_prepare_ustate_all(0, 0); // we don't know which user until later
    lock_take();
    cstate_ty *cstate_data = cp->cstate_get();

    //
    // check they are allowed to
    // (even if edited, may have changed while editing)
    //
    check_permissions(cp, up);

    //
    // change the state
    // remember who reviewed it
    // add to the change's history
    //
    cstate_data->state = cstate_state_being_developed;
    cstate_history_ty *history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_review_fail;
    if (!comment.empty())
        history_data->why = str_copy(comment.get_ref());

    //
    // add it back into the user's change list
    //
    user_ty::pointer devup = user_ty::create(nstring(cp->developer_name()));
    devup->own_add(pp, cp->number);

    //
    // go through the files in the change and unlock them
    // in the baseline
    //
    for (size_t j = 0;; ++j)
    {
        fstate_src_ty   *c_src_data;
        fstate_src_ty   *p_src_data;

        c_src_data = change_file_nth(cp, j, view_path_first);
        if (!c_src_data)
            break;
        p_src_data = pp->file_find(c_src_data->file_name, view_path_none);
        if (!p_src_data)
            continue;
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
                p_src_data->about_to_be_created_by==cid.get_change_number());
            assert(!p_src_data->about_to_be_copied_by ||
                p_src_data->about_to_be_copied_by==cid.get_change_number());
            if (p_src_data->deleted_by)
            {
                assert(!p_src_data->about_to_be_copied_by);
                p_src_data->about_to_be_created_by = 0;
            }
            else
                project_file_remove(pp, c_src_data->file_name);
        }
    }

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
    pp->pstate_write();
    cp->cstate_write();
    devup->ustate_write();
    commit();
    lock_release();

    //
    // run the notify command
    //
    cp->run_review_fail_notify_command();

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(pp, cp);

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("review fail complete"));
    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}


void
review_fail(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, review_fail_help, 0 },
        { arglex_token_list, review_fail_list, 0 },
    };

    trace(("review_fail()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), review_fail_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
