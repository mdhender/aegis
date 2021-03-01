/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994 Peter Miller.
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to implement review pass undo
 */

#include <stdio.h>
#include <ac/stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <ael.h>
#include <aerpu.h>
#include <arglex2.h>
#include <change.h>
#include <commit.h>
#include <dir.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void review_pass_undo_usage _((void));

static void
review_pass_undo_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -Review_Pass_Undo <change_number> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Review_Pass_Undo -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Review_Pass_Undo -Help\n", progname);
	quit(1);
}


static void review_pass_undo_help _((void));

static void
review_pass_undo_help()
{
	static char *text[] =
	{
#include <../man1/aerpu.h>
	};

	help(text, SIZEOF(text), review_pass_undo_usage);
}


static void review_pass_undo_list _((void (*usage)(void)));

static void
review_pass_undo_list(usage)
	void		(*usage)_((void));
{
	string_ty	*project_name;

	trace(("review_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				usage();
			/* fall through... */

		case arglex_token_string:
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
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
	trace((/*{*/"}\n"));
}


static void review_pass_undo_main _((void));

static void
review_pass_undo_main()
{
	cstate		cstate_data;
	pstate		pstate_data;
	cstate_history	history_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;

	trace(("review_pass_undo_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(review_pass_undo_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				review_pass_undo_usage();
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				fatal("duplicate -Change option");
			change_number = arglex_value.alv_number;
			if (change_number < 1)
				fatal("change %ld out of range", change_number);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				review_pass_undo_usage();
			/* fall through... */

		case arglex_token_string:
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * locate user data
	 */
	up = user_executing(pp);

	/*
	 * locate change data
	 */
	if (!change_number)
		change_number = user_default_change(up);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	/*
	 * lock the change for writing
	 */
	change_cstate_lock_prepare(cp);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * it is an error if the change is not in the 'being_reviewed' state.
	 * it is an error if the current user is not the original reviewer
	 */
	if (cstate_data->state != cstate_state_awaiting_integration)
	{
		change_fatal
		(
			cp,
"this change is in the '%s' state, \
it must be in the 'awaiting integration' state to undo a review pass",
			cstate_state_ename(cstate_data->state)
		);
	}
	if (!str_equal(change_reviewer_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
  "user \"%S\" was not the reviewer, only user \"%S\" may undo the review pass",
			user_name(up),
			change_reviewer_name(cp)
		);
	}

	/*
	 * change the state
	 * add to the change's history
	 */
	cstate_data->state = cstate_state_being_reviewed;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_review_pass_undo;

	/*
	 * write out the data and release the locks
	 */
	change_cstate_write(cp);
	commit();
	lock_release();

	/*
	 * run the notify command
	 */
	change_run_review_pass_undo_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, "review pass rescinded");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
review_pass_undo()
{
	trace(("review_pass_undo()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		review_pass_undo_main();
		break;

	case arglex_token_help:
		review_pass_undo_help();
		break;

	case arglex_token_list:
		review_pass_undo_list(review_pass_undo_usage);
		break;
	}
	trace((/*{*/"}\n"));
}
