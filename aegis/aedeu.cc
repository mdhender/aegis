//
// aegis - project change supervisor
// Copyright (C) 1991-1999, 2001-2009, 2011, 2012 Peter Miller
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
#include <libaegis/change/develop_direct/read_write.h>
#include <libaegis/change/identifier.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/col.h>
#include <libaegis/commit.h>
#include <libaegis/common.fmtgen.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/rss.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aedeu.h>


static void
develop_end_undo_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -Undo_Develop_End [ <option>... ]\n", progname);
    fprintf
    (
        stderr,
        "       %s -Undo_Develop_End -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Undo_Develop_End -Help\n", progname);
    quit(1);
}


static void
develop_end_undo_help(void)
{
    help("aedeu", develop_end_undo_usage);
}


static void
develop_end_undo_list(void)
{
    trace(("develop_end_undo_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(develop_end_undo_usage);
    list_changes_in_state_mask
    (
        cid,
        (
            (1 << cstate_state_awaiting_review)
        |
            (1 << cstate_state_being_reviewed)
        |
            (1 << cstate_state_awaiting_integration)
        )
    );
    trace(("}\n"));
}


static void
develop_end_undo_main(void)
{
    trace(("develop_end_undo_main()\n{\n"));
    change_identifier cid;
    arglex();
    string_ty *reason = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(develop_end_undo_usage);
            continue;

        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta_from_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_string:
        case arglex_token_trunk:
            cid.command_line_parse(develop_end_undo_usage);
            continue;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(develop_end_undo_usage);
            break;

        case arglex_token_reason:
            if (reason)
                duplicate_option(develop_end_undo_usage);
            switch (arglex())
            {
            default:
                option_needs_string
                (
                    arglex_token_reason,
                    develop_end_undo_usage
                );
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
    cid.command_line_check(develop_end_undo_usage);

    user_ty::pointer up = cid.get_up();
    user_ty::pointer up_admin;

    //
    // lock the change for writing
    //
    cid.get_pp()->pstate_lock_prepare();
    change_cstate_lock_prepare(cid.get_cp());
    up->ustate_lock_prepare();
    lock_take();
    cstate_ty *cstate_data = cid.get_cp()->cstate_get();

    //
    // Project administrators are allowed to undo the end of development
    // of a branch, no matter who created it.
    //
    if
    (
        cid.get_cp()->was_a_branch()
    &&
        nstring(cid.get_cp()->developer_name()) != up->name()
    &&
        project_administrator_query(cid.get_pp(), up->name())
    )
    {
        up_admin = up;
        up = user_ty::create(nstring(cid.get_cp()->developer_name()));
        // FIXME: lock is now against wrong user
    }

    //
    // If the change is in the being_reviewed state, and the project
    // has been configured to use the awaiting_review state, then nobody
    // may use the aedeu command.
    //
    assert(cstate_data->history->length >= 3);
    if
    (
        cstate_data->history->list[cstate_data->history->length - 1]->what
    ==
        cstate_history_what_review_begin
    )
    {
        assert(cstate_data->state == cstate_state_being_reviewed);
        change_fatal(cid.get_cp(), 0, i18n("bad deu state"));
    }

    //
    // It is an error if the change is not in one of the 'being_reviewed'
    // or 'awaiting_integration' states.
    // It is an error if the current user did not develop the change.
    //
    if
    (
        cstate_data->state != cstate_state_awaiting_review
    &&
        cstate_data->state != cstate_state_being_reviewed
    &&
        cstate_data->state != cstate_state_awaiting_integration
    )
        change_fatal(cid.get_cp(), 0, i18n("bad deu state"));
    if (nstring(cid.get_cp()->developer_name()) != up->name())
        change_fatal(cid.get_cp(), 0, i18n("was not developer"));

    //
    // Change the state.
    // Add to the change's history.
    //
    cstate_data->state = cstate_state_being_developed;
    cstate_history_ty *history_data = change_history_new(cid.get_cp(), up);
    history_data->what = cstate_history_what_develop_end_undo;
    if (up_admin)
    {
        string_ty *r2 =
            str_format
            (
                "Forced by administrator %s.",
                up_admin->name().quote_c().c_str()
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
    // add it back into the user's change list
    //
    up->own_add(cid.get_pp(), cid.get_cp()->number);

    //
    // go through the files in the change and unlock them
    // in the baseline
    //
    for (int j = 0;; ++j)
    {
        fstate_src_ty   *c_src_data;
        fstate_src_ty   *p_src_data;

        c_src_data = change_file_nth(cid.get_cp(), j, view_path_first);
        if (!c_src_data)
            break;
        p_src_data =
            cid.get_pp()->file_find(c_src_data->file_name, view_path_none);
        if (!p_src_data)
        {
            // this is really a corrupted file
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
                project_file_remove(cid.get_pp(), c_src_data->file_name);
        }
    }

    //
    // Make the development directory writable again.
    //
    // This is actually conditional upon project_protect_development_
    // directory_get(cid.get_pp()) but the test is inside the change_
    // development_directory_chmod_read_write(cid.get_cp()) function, because it
    // also makes sure the source files are readable and writable by the
    // developer.
    //
    change_development_directory_chmod_read_write(cid.get_cp());

    //
    // write out the data and release the locks
    //
    cid.get_cp()->cstate_write();
    cid.get_pp()->pstate_write();
    up->ustate_write();
    commit();
    lock_release();

    //
    // run the notify command
    //
    cid.get_cp()->run_develop_end_undo_notify_command();

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(cid.get_pp(), cid.get_cp());

    //
    // verbose success message
    //
    change_verbose(cid.get_cp(), 0, i18n("develop end undo complete"));
    trace(("}\n"));
}


void
develop_end_undo(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, develop_end_undo_help, 0 },
        { arglex_token_list, develop_end_undo_list, 0 },
    };

    trace(("develop_end_undo()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), develop_end_undo_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
