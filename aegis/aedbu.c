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
 * MANIFEST: functions to implement develop begin undo
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <aedbu.h>
#include <ael.h>
#include <arglex2.h>
#include <change.h>
#include <col.h>
#include <commit.h>
#include <common.h>
#include <dir.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void develop_begin_undo_usage _((void));

static void
develop_begin_undo_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -Develop_Begin_Undo <change_number> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Develop_Begin_Undo -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Develop_Begin_Undo -Help\n", progname);
	quit(1);
}


static void develop_begin_undo_help _((void));

static void
develop_begin_undo_help()
{
	static char *text[] =
	{
#include <../man1/aedbu.h>
	};

	help(text, SIZEOF(text), develop_begin_undo_usage);
}


static void develop_begin_undo_list _((void));

static void
develop_begin_undo_list()
{
	string_ty	*project_name;

	trace(("develop_begin_undo_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_begin_undo_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				develop_begin_undo_usage();
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
		1 << cstate_state_being_developed
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void develop_begin_undo_main _((void));

static void
develop_begin_undo_main()
{
	string_ty	*project_name;
	long		change_number;
	project_ty	*pp;
	user_ty		*up;
	change_ty	*cp;
	pstate		pstate_data;
	cstate		cstate_data;
	cstate_history	history_data;
	string_ty	*dd;

	trace(("develop_begin_undo_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_begin_undo_usage);
			continue;

		case arglex_token_keep:
		case arglex_token_interactive:
		case arglex_token_no_keep:
			user_delete_file_argument();
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				develop_begin_undo_usage();
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
				develop_begin_undo_usage();
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
	 * Take an advisory write lock on the appropriate row of the change
	 * table.  Take an advisory write lock on the appropriate row of the
	 * user table.  Block until can get both simultaneously.
	 */
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	user_ustate_lock_prepare(up);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * It is an error if the change is not in the being developed state.
	 * It is an error if the change is not assigned to the current user.
	 */
	if (cstate_data->state != cstate_state_being_developed)
	{
		change_fatal
		(
			cp,
"this change is in the '%s' state, \
it must be in the 'being developed' state to undo develop begin",
			cstate_state_ename(cstate_data->state)
		);
	}
	if (!str_equal(change_developer_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
    "user \"%S\" is not the developer, only user \"%S\" may undo develop begin",
			user_name(up),
			change_developer_name(cp)
		);
	}

	/*
	 * add to history for state change
	 */
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_develop_begin_undo;

	/*
	 * Send the change to the awaiting-development state.
	 * Clear the build-time field.
	 * Clear the test-time field.
	 * Clear the test-baseline-time field.
	 * Clear the src field.
	 */
	cstate_data->state = cstate_state_awaiting_development;
	change_build_times_clear(cp);
	while (cstate_data->src->length)
		change_src_remove(cp, cstate_data->src->list[0]->file_name);

	/*
	 * Remove the change from the list of assigned changes in the user
	 * change table (in the user row).
	 */
	user_own_remove(up, project_name_get(pp), change_number);

	/*
	 * remove the development directory
	 */
	dd = change_development_directory_get(cp, 1);
	if (user_delete_file_query(up, dd, 1))
	{
		change_verbose(cp, "remove development directory");
		user_become(up);
		commit_rmdir_tree_errok(dd);
		user_become_undo();
	}

	/*
	 * clear development directory field
	 */
	change_development_directory_clear(cp);

	/*
	 * Write the change table row.
	 * Write the user table row.
	 * Release advisory locks.
	 */
	change_cstate_write(cp);
	project_pstate_write(pp);
	user_ustate_write(up);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	change_verbose(cp, "no longer being developed");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
develop_begin_undo()
{
	trace(("develop_begin_undo()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		develop_begin_undo_main();
		break;

	case arglex_token_help:
		develop_begin_undo_help();
		break;

	case arglex_token_list:
		develop_begin_undo_list();
		break;
	}
	trace((/*{*/"}\n"));
}
