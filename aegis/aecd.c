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
 * MANIFEST: functions to change directory or determine paths
 */

#include <stdio.h>
#include <ac/stdlib.h>

#include <aecd.h>
#include <ael.h>
#include <arglex2.h>
#include <change.h>
#include <error.h>
#include <help.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <user.h>


static void change_directory_usage _((void));

static void
change_directory_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -Change_Directory [ <option>... ][ <subdir> ]\n", progname);
	fprintf(stderr, "       %s -Change_Directory -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Change_Directory -Help\n", progname);
	quit(1);
}


static void change_directory_help _((void));

static void
change_directory_help()
{
	static char *text[] =
	{
#include <../man1/aecd.h>
	};

	help(text, SIZEOF(text), change_directory_usage);
}


static void change_directory_list _((void));

static void
change_directory_list()
{
	string_ty	*project_name;

	trace(("change_directory_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(change_directory_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				change_directory_usage();
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
		(
			(1 << cstate_state_being_developed)
		|
			(1 << cstate_state_being_reviewed)
		|
			(1 << cstate_state_awaiting_integration)
		|
			(1 << cstate_state_being_integrated)
		)
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void change_directory_main _((void));

static void
change_directory_main()
{
	char		*subdir = 0;
	int		devdir = 0;
	cstate		cstate_data;
	string_ty	*d;
	int		baseline = 0;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;

	trace(("change_directory_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(change_directory_usage);
			continue;

		case arglex_token_string:
			if (subdir)
				fatal("too many subdirectories specified");
			subdir = arglex_value.alv_string;
			if (!*subdir || *subdir == '/')
				fatal("subdirectory must be relative");
			break;

		case arglex_token_development_directory:
			if (devdir)
			       fatal("duplicate -Develompent_Directory option");
			if (baseline)
			{
				bad_combo:
				fatal
				(
	     "only one of -BaseLine and -Development_Directory may be specified"
				);
			}
			devdir = 1;
			break;

		case arglex_token_baseline:
			if (baseline)
				fatal("duplicate -BaseLine option");
			if (devdir)
				goto bad_combo;
			baseline = 1;
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				change_directory_usage();
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
				change_directory_usage();
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
	 * figure out where to go
	 */
	if (baseline)
	{
		if (change_number)
		{
			fatal
			(
		      "the -BaseLine and -Change options are mutually exclusive"
			);
		}
		d = project_baseline_path_get(pp, 0);
		cp = 0;
	}
	else
	{
		/*
		 * locate change data
		 */
		if (!change_number)
			change_number = user_default_change(up);
		cp = change_alloc(pp, change_number);
		change_bind_existing(cp);

		cstate_data = change_cstate_get(cp);
		switch (cstate_data->state)
		{
		default:
			change_fatal
			(
				cp,
	     "this change is in the '%s' state, there is no directory to go to",
				cstate_state_ename(cstate_data->state)
			);

		case cstate_state_being_integrated:
			if (!devdir)
			{
				d = change_integration_directory_get(cp, 0);
				break;
			}
			/* fall through... */

		case cstate_state_awaiting_integration:
		case cstate_state_being_reviewed:
		case cstate_state_being_developed:
			d = change_development_directory_get(cp, 0);
			break;
		}
	}

	/*
	 * Add in the extra path elements as necessary.
	 * Flatten it out if they go up the tree (etc).
	 */
	if (subdir)
	{
		string_ty *tmp;

		tmp = str_format("%S/%s", d, subdir);
		user_become(up);
		d = os_pathname(tmp, 0);
		user_become_undo();
		str_free(tmp);
	}

	/*
	 * print out the path
	 */
	printf("%s\n", d->str_text);
	if (!cp)
		project_verbose(pp, "%s", d->str_text);
	else
	{
		change_verbose(cp, "%s", d->str_text);
		change_free(cp);
	}
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
change_directory()
{
	trace(("change_directory()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		change_directory_main();
		break;

	case arglex_token_help:
		change_directory_help();
		break;

	case arglex_token_list:
		change_directory_list();
		break;
	}
	trace((/*{*/"}\n"));
}
