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
 * MANIFEST: functions to implement new change undo
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/time.h>

#include <aeca.h>
#include <ael.h>
#include <aencu.h>
#include <arglex2.h>
#include <cattr.h>
#include <change.h>
#include <col.h>
#include <commit.h>
#include <common.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <user.h>


static void new_change_undo_usage _((void));

static void
new_change_undo_usage()
{
	char	*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -New_Change_Undo [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Change_Undo -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Change_Undo -Help\n", progname);
	quit(1);
}


static void new_change_undo_help _((void));

static void
new_change_undo_help()
{
	static char *text[] =
	{
#include <../man1/aencu.h>
	};

	help(text, SIZEOF(text), new_change_undo_usage);
}


static void new_change_undo_list _((void));

static void
new_change_undo_list()
{
	string_ty	*project_name;

	trace(("new_change_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_change_undo_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				new_change_undo_usage();
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
		1 << cstate_state_awaiting_development
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void new_change_undo_main _((void));

static void
new_change_undo_main()
{
	string_ty	*project_name;
	long		change_number;
	project_ty	*pp;
	user_ty		*up;
	change_ty	*cp;
	cstate		cstate_data;
	pstate		pstate_data;

	trace(("new_change_undo_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_change_undo_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				new_change_undo_usage();
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
				new_change_undo_usage();
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
	 *
	 * The change number must be given on the command line,
	 * even if there is only one appropriate change.
	 * The is the "least surprizes" principle at work,
	 * even though we could sometimes work this out for ourself.
	 */
	if (!change_number)
		change_number = user_default_change(up);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	/*
	 * Take an advisory write lock on the project state
	 * and the change state.
	 */
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * Extract the appropriate row of the change table.
	 * It is an error if the change is not in the
	 * awaiting_development state.
	 */
	if (cstate_data->state != cstate_state_awaiting_development)
	{
		change_fatal
		(
			cp,
"this change is in the '%s' state, \
it must be in the 'awaiting development' state to undo a new change",
			cstate_state_ename(cstate_data->state)
		);
	}
	if (!project_administrator_query(pp, user_name(up)))
	{
		project_fatal
		(
			pp,
			"user \"%S\" is not an administrator",
			user_name(up)
		);
	}

	/*
	 * tell the project to forget this change
	 */
	project_change_delete(pp, change_number);

	/*
	 * delete the change state file
	 */
	project_become(pp);
	commit_unlink_errok(cp->filename);
	project_become_undo();

	/*
	 * Update change table row (and change history table).
	 * Update user table row.
	 * Release advisory write locks.
	 */
	project_pstate_write(pp);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	change_verbose(cp, "removed");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
new_change_undo()
{
	trace(("new_change_undo()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		new_change_undo_main();
		break;

	case arglex_token_help:
		new_change_undo_help();
		break;

	case arglex_token_list:
		new_change_undo_list();
		break;
	}
	trace((/*{*/"}\n"));
}
