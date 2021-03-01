//
// aegis - project change supervisor
// Copyright (C) 1991-1999, 2001-2009, 2012 Peter Miller
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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/time.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

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
#include <libaegis/col.h>
#include <libaegis/commit.h>
#include <libaegis/common.fmtgen.h>
#include <libaegis/dir.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/rss.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aedbu.h>


static void
develop_begin_undo_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -Develop_Begin_Undo <change_number> [ <option>... ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -Develop_Begin_Undo -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Develop_Begin_Undo -Help\n", progname);
    quit(1);
}


static void
develop_begin_undo_help(void)
{
    help("aedbu", develop_begin_undo_usage);
}


static void
develop_begin_undo_list(void)
{
    trace(("develop_begin_undo_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(develop_begin_undo_usage);
    list_changes_in_state_mask(cid, 1 << cstate_state_being_developed);
    trace(("}\n"));
}


static void
develop_begin_undo_main(void)
{
    cstate_ty       *cstate_data;
    cstate_history_ty *history_data;

    trace(("develop_begin_undo_main()\n{\n"));
    arglex();
    string_ty *reason = 0;
    bool new_change_undo = false;
    change_identifier cid;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(develop_begin_undo_usage);
            continue;

        case arglex_token_change:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_string:
            cid.command_line_parse(develop_begin_undo_usage);
            continue;

        case arglex_token_keep:
        case arglex_token_interactive:
        case arglex_token_keep_not:
            user_ty::delete_file_argument(develop_begin_undo_usage);
            break;

        case arglex_token_user:
            if (arglex() != arglex_token_string)
                option_needs_name(arglex_token_user, develop_begin_undo_usage);
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(develop_begin_undo_usage);
            break;

        case arglex_token_reason:
            if (reason)
                duplicate_option(develop_begin_undo_usage);
            switch (arglex())
            {
            default:
                option_needs_string
                (
                    arglex_token_reason,
                    develop_begin_undo_usage
                );
                // NOTREACHED

            case arglex_token_string:
            case arglex_token_number:
                reason = str_from_c(arglex_value.alv_string);
                break;
            }
            break;

        case arglex_token_new_change_undo:
            if (new_change_undo)
                duplicate_option(develop_begin_undo_usage);
            new_change_undo = true;
            break;
        }
        arglex();
    }
    cid.command_line_check(develop_begin_undo_usage);

    //
    // Make sure it is a change we are manipulating, not a branch.
    //
    if (cid.get_cp()->is_a_branch())
        change_fatal(cid.get_cp(), 0, i18n("use aenbru instead"));

    //
    // Take an advisory write lock on the appropriate row of the change
    // table.  Take an advisory write lock on the appropriate row of the
    // user table.  Block until can get both simultaneously.
    //
    cid.get_pp()->pstate_lock_prepare();
    change_cstate_lock_prepare(cid.get_cp());
    cid.get_cp()->developer_user_get()->ustate_lock_prepare();
    lock_take();
    cstate_data = cid.get_cp()->cstate_get();

    bool is_admin =
        project_administrator_query(cid.get_pp(), cid.get_up()->name());
    bool is_developer =
        nstring(cid.get_cp()->developer_name()) == cid.get_up()->name();
    bool is_creator =
        nstring(cid.get_cp()->creator_name()) == cid.get_up()->name();

    //
    // It is an error if the change is not in the being developed state.
    // It is an error if the change is not assigned to the current user.
    //
    if (cstate_data->state != cstate_state_being_developed)
        change_fatal(cid.get_cp(), 0, i18n("bad dbu state"));
    if (!is_admin && !is_developer)
        change_fatal(cid.get_cp(), 0, i18n("not developer"));
    if (new_change_undo && !is_admin && !is_creator)
        project_fatal(cid.get_pp(), 0, i18n("not an administrator"));

    //
    // It is an error if an administrator is nuking the change and
    // the change has any files.  Mostly, because it is expected
    // that this is used to undo forced aedb's.
    // (This guards against admin typos, too.)
    //
    if
    (
        is_admin
    &&
        !is_developer
    &&
        change_file_nth(cid.get_cp(), 0L, view_path_first)
    )
        change_fatal(cid.get_cp(), 0, i18n("aedbu, has files"));

    //
    // remove the development directory
    //
    nstring dd(change_development_directory_get(cid.get_cp(), 1));
    if (cid.get_cp()->developer_user_get()->delete_file_query(dd, true, -1))
    {
        change_verbose(cid.get_cp(), 0, i18n("remove development directory"));
        user_ty::become scoped(cid.get_cp()->developer_user_get());
        commit_rmdir_tree_errok(dd);
    }

    if (new_change_undo)
    {
        //
        // tell the project to forget this change
        //
        project_change_delete(cid.get_pp(), cid.get_cp()->number);

        //
        // delete the change state file
        //
        project_become(cid.get_pp());
        commit_unlink_errok(cid.get_cp()->cstate_filename_get());
        commit_unlink_errok(change_fstate_filename_get(cid.get_cp()));
        project_become_undo(cid.get_pp());
    }
    else
    {
        //
        // add to history for state change
        //
        history_data =
            change_history_new
            (
                cid.get_cp(),
                cid.get_cp()->developer_user_get()
            );
        history_data->what = cstate_history_what_develop_begin_undo;
        if (is_admin && !is_developer)
        {
            string_ty *r2 =
                str_format
                (
                    "Forced by administrator %s.",
                    cid.get_up()->name().quote_c().c_str()
                );
            if (reason)
            {
                string_ty *r1 = reason;
                reason = str_format("%s\n%s", r1->str_text, r2->str_text);
                str_free(r1);
                str_free(r2);
            }
            else
                reason = r2;
        }
        history_data->why = reason;

        //
        // Send the change to the awaiting-development state.
        // Clear the build-time field.
        // Clear the test-time field.
        // Clear the test-baseline-time field.
        // Clear the src field.
        //
        cstate_data->state = cstate_state_awaiting_development;
        change_build_times_clear(cid.get_cp());
        change_file_remove_all(cid.get_cp());

        //
        // clear development directory field
        //
        change_development_directory_clear(cid.get_cp());

        //
        // Write the change table row.
        //
        cid.get_cp()->cstate_write();
    }

    //
    // Remove the change from the list of assigned changes in the user
    // change table (in the user row).
    //
    {
        user_ty::pointer devu = cid.get_cp()->developer_user_get();
        devu->own_remove(cid.get_pp(), cid.get_cp()->number);
        devu->ustate_write();
    }

    cid.get_pp()->pstate_write();
    commit();
    lock_release();

    if (!new_change_undo)
    {
        //
        // run the notification command
        //
        change_run_develop_begin_undo_command
        (
            cid.get_cp(),
            cid.get_cp()->developer_user_get()
        );

        //
        // Update the RSS feed file if necessary.
        //
        rss_add_item_by_change(cid.get_pp(), cid.get_cp());
    }

    //
    // verbose success message
    //
    change_verbose(cid.get_cp(), 0, i18n("develop begin undo complete"));
    if (new_change_undo)
        change_verbose(cid.get_cp(), 0, i18n("new change undo complete"));
    trace(("}\n"));
}


void
develop_begin_undo(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, develop_begin_undo_help, 0 },
        { arglex_token_list, develop_begin_undo_list, 0 },
    };

    trace(("develop_begin_undo()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), develop_begin_undo_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
