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
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/dir.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/rss.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aerpu.h>


static void
review_pass_undo_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -Review_Pass_Undo <change_number> [ <option>... ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -Review_Pass_Undo -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Review_Pass_Undo -Help\n", progname);
    quit(1);
}


static void
review_pass_undo_help(void)
{
    help("aerpu", review_pass_undo_usage);
}


static void
review_pass_undo_list(void)
{
    trace(("review_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(review_pass_undo_usage);
    list_changes_in_state_mask(cid, 1 << cstate_state_awaiting_integration);
    trace(("}\n"));
}


static void
review_pass_undo_main(void)
{
    trace(("review_pass_undo_main()\n{\n"));
    arglex();
    change_identifier cid;
    string_ty *reason = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(review_pass_undo_usage);
            continue;

        case arglex_token_change:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_string:
            cid.command_line_parse(review_pass_undo_usage);
            continue;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(review_pass_undo_usage);
            break;

        case arglex_token_reason:
            if (reason)
                duplicate_option(review_pass_undo_usage);
            switch (arglex())
            {
            default:
                option_needs_string
                (
                    arglex_token_reason,
                    review_pass_undo_usage
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
    cid.command_line_check(review_pass_undo_usage);

    project *pp = cid.get_pp();
    user_ty::pointer up = cid.get_up();
    change::pointer cp = cid.get_cp();

    //
    // lock the change for writing
    //
    change_cstate_lock_prepare(cp);
    lock_take();
    cstate_ty *cstate_data = cp->cstate_get();

    //
    // It is an error if the change is not in the 'awaiting
    // integration' state.  It is an error if the current user is
    // not the original reviewer
    //
    if (!change_reviewer_already(cp, up->name()))
        change_fatal(cp, 0, i18n("was not reviewer"));
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_integration:
        break;

    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
        //
        // Depending on the settings of the develop_en_action fields and
        // the review_policy_command filed, the change could also be in one
        // of these states, as well.
        //
        break;

    case cstate_state_awaiting_development:
    case cstate_state_being_developed:
        //
        // Becasue the change_reviewer_already function returned true
        // for us to get here, these two states are supposed to be
        // impossible.
        //
        assert(0);
        // fall through...

    case cstate_state_being_integrated:
    case cstate_state_completed:
#ifndef DEBUG
    default:
#endif
        //
        // Complain if they try to rescind a review too late in the
        // process.
        //
        change_fatal(cp, 0, i18n("bad rpu state"));
        // NOTREACHED
    }

    //
    // change the state
    // add to the change's history
    //
    cstate_data->state = cstate_state_being_reviewed;
    cstate_history_ty *history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_review_pass_undo;
    history_data->why = reason;

    //
    // write out the data and release the locks
    //
    cp->cstate_write();
    commit();
    lock_release();

    //
    // run the notify command
    //
    cp->run_review_pass_undo_notify_command();

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(pp, cp);

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("review pass undo complete"));
    trace(("}\n"));
}


void
review_pass_undo(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, review_pass_undo_help, 0 },
        { arglex_token_list, review_pass_undo_list, 0 },
    };

    trace(("review_pass_undo()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), review_pass_undo_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
