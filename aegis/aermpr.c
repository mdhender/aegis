/*
 *	aegis - project change supervisor
 *	Copyright (C) 1993, 1994 Peter Miller.
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
 * MANIFEST: functions to implement remove project
 */

#include <stdio.h>

#include <ael.h>
#include <aermpr.h>
#include <arglex2.h>
#include <change.h>
#include <commit.h>
#include <error.h>
#include <gonzo.h>
#include <help.h>
#include <lock.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <str.h>
#include <trace.h>
#include <user.h>


static void remove_project_usage _((void));

static void
remove_project_usage()
{
	char	*progname;

	progname = option_progname_get();
	fprintf
	(
		stderr,
		"usage: %s -ReMove_PRoject [ <option>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -ReMove_PRoject -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -ReMove_PRoject -Help\n", progname);
	quit(1);
}


static void remove_project_help _((void));

static void
remove_project_help()
{
	static char *text[] =
	{
#include <../man1/aermpr.h>
	};

	help(text, SIZEOF(text), remove_project_usage);
}


static void remove_project_list _((void));

static void
remove_project_list()
{
	arglex();
	while (arglex_token != arglex_token_eoln)
		generic_argument(remove_project_usage);
	list_projects(0, 0);
}


static void remove_project_main _((void));

static void
remove_project_main()
{
	long		nerr;
	int		j;
	pstate		pstate_data;
	string_ty	*project_name;
	project_ty	*pp;
	change_ty	*cp;
	cstate		cstate_data;
	user_ty		*up;
	int		still_exists;

	trace(("remove_project_main()\n{\n"/*}*/));
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(remove_project_usage);
			continue;

		case arglex_token_keep:
		case arglex_token_interactive:
		case arglex_token_no_keep:
			user_delete_file_argument();
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				remove_project_usage();
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
		fatal("project must be named explicitly");
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * see if the user already deleted it
	 */
	os_become_orig();
	still_exists = os_exists(project_pstate_path_get(pp));
	os_become_undo();

	/*
	 * locate user data
	 */
	up = user_executing(still_exists ? pp : (project_ty *)0);

	/*
	 * lock the project
	 */
	project_pstate_lock_prepare(pp);
	gonzo_gstate_lock_prepare_new();
	lock_take();

	/*
	 * avoid reading any project state information
	 * if it is already gone
	 */
	if (!still_exists)
		goto nuke;

	/*
	 * it is an error if any of the changes are active
	 */
	pstate_data = project_pstate_get(pp);
	nerr = 0;
	for (j = 0; j < pstate_data->change->length; ++j)
	{
		long	change_number;

		change_number = pstate_data->change->list[j];
		cp = change_alloc(pp, change_number);
		change_bind_existing(cp);
		cstate_data = change_cstate_get(cp);
		if
		(
			cstate_data->state >= cstate_state_being_developed
		&& 
			cstate_data->state <= cstate_state_being_integrated
		)
		{
			change_error(cp, "still active");
			++nerr;
		}
		change_free(cp);
	}

	/*
	 * it is an error if the current user is not an administrator
	 */
	if (!project_administrator_query(pp, user_name(up)))
	{
		project_error
		(
			pp,
			"user \"%S\" is not an administrator",
			user_name(up)
		);
		nerr++;
	}
	if (nerr)
		quit(1);

	/*
	 * remove the project directory
	 */
	if (user_delete_file_query(up, project_home_path_get(pp), 1))
	{
		project_verbose(pp, "remove project directory");
		project_become(pp);
		commit_rmdir_tree_errok(project_home_path_get(pp));
		project_become_undo();
	}

	/*
	 * tell gonzo to forget about this project
	 */
	nuke:
	gonzo_project_delete(pp);
	gonzo_gstate_write();

	/*
	 * release the locks
	 */
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	project_verbose(pp, "removed");

	/*
	 * clean up and go home
	 */
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
remove_project()
{
	trace(("remove_project()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		remove_project_main();
		break;

	case arglex_token_help:
		remove_project_help();
		break;

	case arglex_token_list:
		remove_project_list();
		break;
	}
	trace((/*{*/"}\n"));
}
