//
//	aegis - project change supervisor
//	Copyright (C) 1991-1999, 2001-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to implement review pass undo
//

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/sys/types.h>
#include <sys/stat.h>

#include <ael/change/by_state.h>
#include <aerpu.h>
#include <arglex2.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <change.h>
#include <commit.h>
#include <dir.h>
#include <error.h> // for assert
#include <file.h>
#include <help.h>
#include <lock.h>
#include <mem.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <quit.h>
#include <rss.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


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
    string_ty	    *project_name;

    trace(("review_list()\n{\n"));
    project_name = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(review_pass_undo_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    // fall through...

	case arglex_token_string:
	    arglex_parse_project(&project_name, review_pass_undo_usage);
	    continue;
	}
	arglex();
    }
    list_changes_in_state_mask
    (
	project_name,
	1 << cstate_state_awaiting_integration
    );
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


static void
review_pass_undo_main(void)
{
    cstate_ty	    *cstate_data;
    cstate_history_ty *history_data;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    user_ty	    *up;

    trace(("review_pass_undo_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    string_ty *reason = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(review_pass_undo_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change
	    (
		&project_name,
		&change_number,
		review_pass_undo_usage
	    );
	    continue;

	case arglex_token_project:
	    arglex();
	    // fall through...

	case arglex_token_string:
	    arglex_parse_project(&project_name, review_pass_undo_usage);
	    continue;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(review_pass_undo_usage);
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

    //
    // locate project data
    //
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    //
    // locate user data
    //
    up = user_executing(pp);

    //
    // locate change data
    //
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // lock the change for writing
    //
    change_cstate_lock_prepare(cp);
    lock_take();
    cstate_data = change_cstate_get(cp);

    //
    // It is an error if the change is not in the 'awaiting
    // integration' state.  It is an error if the current user is
    // not the original reviewer
    //
    if (!change_reviewer_already(cp, user_name(up)))
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
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_review_pass_undo;
    history_data->why = reason;

    //
    // write out the data and release the locks
    //
    change_cstate_write(cp);
    commit();
    lock_release();

    //
    // run the notify command
    //
    change_run_review_pass_undo_notify_command(cp);

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(pp, cp);

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("review pass undo complete"));
    change_free(cp);
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}


void
review_pass_undo(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, review_pass_undo_help, },
	{arglex_token_list, review_pass_undo_list, },
    };

    trace(("review_pass_undo()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), review_pass_undo_main);
    trace(("}\n"));
}
