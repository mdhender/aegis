//
// aegis - project change supervisor
// Copyright (C) 2001-2008, 2011, 2012 Peter Miller
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
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/dir.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/project/history.h>
#include <libaegis/rss.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aerbu.h>


static void
review_begin_undo_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -Review_PASS <change_number> [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Review_PASS -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Review_PASS -Help\n", progname);
    quit(1);
}


static void
review_begin_undo_help(void)
{
    help("aerbu", review_begin_undo_usage);
}


static void
review_begin_undo_list(void)
{
    trace(("review_begin_undo_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(review_begin_undo_usage);
    list_changes_in_state_mask(cid, 1 << cstate_state_being_reviewed);
    trace(("}\n"));
}


static void
review_begin_undo_main(void)
{
    cstate_ty       *cstate_data;
    cstate_history_ty *history_data;
    string_ty       *project_name;
    project      *pp;
    long            change_number;
    change::pointer cp;
    user_ty::pointer up;

    trace(("review_begin_undo_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    string_ty *reason = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(review_begin_undo_usage);
            continue;

        case arglex_token_change:
            arglex();
            // fall through...

        case arglex_token_number:
            arglex_parse_change
            (
                &project_name,
                &change_number,
                review_begin_undo_usage
            );
            continue;

        case arglex_token_project:
            arglex();
            // fall through...

        case arglex_token_string:
            arglex_parse_project(&project_name, review_begin_undo_usage);
            continue;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(review_begin_undo_usage);
            break;

        case arglex_token_reason:
            if (reason)
            duplicate_option(review_begin_undo_usage);
            switch (arglex())
            {
            default:
                option_needs_string
                (
                    arglex_token_reason,
                    review_begin_undo_usage
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
    // lock the change for writing
    //
    change_cstate_lock_prepare(cp);
    lock_take();
    cstate_data = cp->cstate_get();

    //
    // it is an error if the change is not in the 'being_reviewed' state.
    //
    if (cstate_data->state != cstate_state_being_reviewed)
        change_fatal(cp, 0, i18n("bad rbu state"));
    if
    (
        project_develop_end_action_get(pp)
    !=
        pattr_develop_end_action_goto_awaiting_review
    )
    {
        if (!project_reviewer_query(pp, up->name()))
            project_fatal(pp, 0, i18n("not a reviewer"));
        if
        (
            !project_developer_may_review_get(pp)
        &&
            nstring(cp->developer_name()) == up->name()
        )
            change_fatal(cp, 0, i18n("developer may not review"));
    }
    else
    {
        if (nstring(cp->reviewer_name()) != up->name())
            change_fatal(cp, 0, i18n("not reviewer"));

        //
        // change the state
        // remember who reviewed it
        // add to the change's history
        //
        cstate_data->state = cstate_state_awaiting_review;
        history_data = change_history_new(cp, up);
        history_data->what = cstate_history_what_review_begin_undo;
        history_data->why = reason;
    }

    //
    // write out the data and release the locks
    //
    cp->cstate_write();
    commit();
    lock_release();

    //
    // run the notify command
    //
    cp->run_review_begin_undo_notify_command();

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(pp, cp);

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("review begin undo complete"));
    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}


void
review_begin_undo(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, review_begin_undo_help, 0 },
        { arglex_token_list, review_begin_undo_list, 0 },
    };

    trace(("review_begin_undo()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), review_begin_undo_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
