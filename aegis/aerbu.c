/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate aerbus
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <ael/change/by_state.h>
#include <aerbu.h>
#include <arglex2.h>
#include <change.h>
#include <change/file.h>
#include <commit.h>
#include <dir.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <mem.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <project_hist.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void review_begin_undo_usage _((void));

static void
review_begin_undo_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf(stderr, "usage: %s -Review_PASS <change_number> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Review_PASS -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Review_PASS -Help\n", progname);
	quit(1);
}


static void review_begin_undo_help _((void));

static void
review_begin_undo_help()
{
	help("aerbu", review_begin_undo_usage);
}


static void review_begin_undo_list _((void));

static void
review_begin_undo_list()
{
	string_ty	*project_name;

	trace(("review_begin_undo_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(review_begin_undo_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, review_begin_undo_usage);
			/* fall through... */

		case arglex_token_string:
			if (project_name)
				duplicate_option_by_name(arglex_token_project, review_begin_undo_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_changes_in_state_mask
	(
		project_name,
		1 << cstate_state_being_reviewed
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void review_begin_undo_main _((void));

static void
review_begin_undo_main()
{
	cstate		cstate_data;
	cstate_history	history_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;

	trace(("review_begin_undo_main()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(review_begin_undo_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, review_begin_undo_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, review_begin_undo_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set_long(scp, "Number", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, review_begin_undo_usage);
			/* fall through... */

		case arglex_token_string:
			if (project_name)
				duplicate_option_by_name(arglex_token_project, review_begin_undo_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(review_begin_undo_usage);
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

	/*
	 * it is an error if the change is not in the 'being_reviewed' state.
	 */
	if (cstate_data->state != cstate_state_being_reviewed)
		change_fatal(cp, 0, i18n("bad rbu state"));
	if (project_develop_end_action_get(pp) !=
		pattr_develop_end_action_goto_awaiting_review)
	{
		if (!project_reviewer_query(pp, user_name(up)))
			project_fatal(pp, 0, i18n("not a reviewer"));
		if
		(
			!project_developer_may_review_get(pp)
		&&
			str_equal(change_developer_name(cp), user_name(up))
		)
			change_fatal(cp, 0, i18n("developer may not review"));
	}
	else
	{
		if (!str_equal(change_reviewer_name(cp), user_name(up)))
			change_fatal(cp, 0, i18n("not reviewer"));

		/*
		 * change the state
		 * remember who reviewed it
		 * add to the change's history
		 */
		cstate_data->state = cstate_state_awaiting_review;
		history_data = change_history_new(cp, up);
		history_data->what = cstate_history_what_review_begin_undo;
	}

	/*
	 * write out the data and release the locks
	 */
	change_cstate_write(cp);
	commit();
	lock_release();

	/*
	 * run the notify command
	 */
	change_run_review_begin_undo_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, 0, i18n("review begin undo complete"));
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
review_begin_undo()
{
	static arglex_dispatch_ty dispatch[] =
	{
		{ arglex_token_help,		review_begin_undo_help,	},
		{ arglex_token_list,		review_begin_undo_list,	},
	};

	trace(("review_begin_undo()\n{\n"));
	arglex_dispatch(dispatch, SIZEOF(dispatch), review_begin_undo_main);
	trace(("}\n"));
}
