/*
 *	aegis - project change supervisor
 *	Copyright (C) 1993 Peter Miller.
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
"NAME",
"	%s -ReMove_PRoject - remove project",
"",
"SYNOPSIS",
"	%s -ReMove_Project <project-name> [ <option>...  ]",
"	%s -ReMove_Project -List [ <option>...  ]",
"	%s -ReMove_Project -Help",
"",
"DESCRIPTION",
"	The %s -ReMove_PRoject command is used to remove a",
"	project, either entirely, or just from %s' supervision.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Keep",
"		This option may be used to retain files and/or",
"		directories usually deleted by the command.",
"",
"	-LIBrary <abspath>",
"		This option may be used to specify a directory to be",
"		searched for global state files and user state",
"		files.  (See aegstate(5) and aeustate(5) for more",
"		information.) Several library options may be present",
"		on the command line, and are search in the order",
"		given.  Appended to this explicit search path are",
"		the directories specified by the AEGIS enviroment",
"		variable (colon separated), and finally,",
"		/usr/local/lib/%s is always searched.  All paths",
"		specified, either on the command line or in the",
"		AEGIS environment variable, must be absolute.",
"",
"	-List",
"		This option may be used to obtain a list of suitable",
"		subjects for this command.  The list may be more",
"		general than expected.",
"",
"	-Help",
"		This option may be used to obtain more information",
"		about how to use the %s program.",
"",
"	-Project <name>",
"		This option may be used to select the project of",
"		interest.  When no -Project option is specified, the",
"		AEGIS_PROJECT environment variable is consulted.  If",
"		that does not exist, the user's $HOME/.aegisrc file",
"		is examined for a default project field (see",
"		aeuconf(5) for more information).  If that does not",
"		exist, when the user is only working on changes",
"		within a single project, the project name defaults",
"		to that project.  Otherwise, it is an error.",
"",
"	-TERse",
"		This option may be used to cause listings to produce",
"		the bare minimum of information.  It is usually",
"		useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause %s to produce",
"		more output.  By default %s only produces output",
"		on errors.  When used with the -List option this",
"		option causes column headings to be added.",
"",
"	All options may be abbreviated; the abbreviation is",
"	documented as the upper case letters, all lower case",
"	letters and underscores (_) are optional.  You must use",
"	consecutive sequences of optional letters.",
"",
"	All options are case insensitive, you may type them in",
"	upper case or lower case or a combination of both, case",
"	is not important.",
"",
"	For example: the arguments \"-project, \"-PROJ\" and \"-p\"",
"	are all interpreted to mean the -Project option.  The",
"	argument \"-prj\" will not be understood, because",
"	consecutive optional characters were not supplied.",
"",
"	Options and other command line arguments may be mixed",
"	arbitrarily on the command line, after the function",
"	selectors.",
"",
"	The GNU long option names are understood.  Since all",
"	option names for aegis are long, this means ignoring the",
"	extra leading '-'.  The \"--option=value\" convention is",
"	also understood.",
"",
"RECOMMENDED ALIAS",
"	The recommended alias for this command is",
"	csh%%	alias aermpr '%s -rmpr \\!* -v'",
"	sh$	aermpr(){%s -rmpr $* -v}",
"",
"ERRORS",
"	It is an error if the project has any changes between the",
"	being developed and being integrated states, inclusive.",
"	It is an error if the current user is not an administrator.",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any error.",
"	The %s command will only exit with a status of 0 if there",
"	are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
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
	int		keep;

	trace(("remove_project_main()\n{\n"/*}*/));
	keep = 0;
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(remove_project_usage);
			continue;

		case arglex_token_keep:
			if (keep)
			{
				error
				(
					"duplicate \"%s\" option",
					arglex_value.alv_string
				);
				remove_project_usage();
			}
			keep = 1;
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
	 * locate user data
	 */
	up = user_executing(pp);

	/*
	 * lock the project
	 */
	project_pstate_lock_prepare(pp);
	gonzo_gstate_lock_prepare_new();
	lock_take();
	pstate_data = project_pstate_get(pp);

	/*
	 * it is an error if any of the changes are active
	 */
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
	if (!keep)
	{
		project_verbose(pp, "remove project directory");
		project_become(pp);
		commit_rmdir_tree_errok(project_home_path_get(pp));
		project_become_undo();
	}

	/*
	 * tell gonzo to forget about this project
	 */
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
