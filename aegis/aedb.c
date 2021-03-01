/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 * MANIFEST: functions to implement develop begin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <aedb.h>
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


static void develop_begin_usage _((void));

static void
develop_begin_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -Develop_Begin <change_number> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Develop_Begin -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Develop_Begin -Help\n", progname);
	quit(1);
}


static void develop_begin_help _((void));

static void
develop_begin_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Develop_Begin - begin development of a change",
"",
"SYNOPSIS",
"	%s -Develop_Begin <change-number> [ <option>... ]",
"	%s -Develop_Begin -List [ <option>... ]",
"	%s -Develop_Begin -Help",
"",
"DESCRIPTION",
"	The %s -Develop_Begin command is used to commence",
"	development of a change.",
"",
"	The development directory for the change will be created",
"	automatically; below the directory specified in the",
"	default_development_directory field of aeuconf(5), or if",
"	not set below the directory specified in the default_-",
"	development_directory field of aepattr(5), or if not set",
"	below the current user's home directory.  It is rare to",
"	need to know the exact pathname of the development",
"	directory, as the aecd(1) command can take you there at",
"	any time.",
"",
"	Successful execution of this command will move the",
"	specified change from the awaiting development state to",
"	the being developed state.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Change <number>",
"		This option may be used to specify a particular",
"		change within a project.  When no -Change option",
"		is specified, the AEGIS_CHANGE environment",
"		variable is consulted.  If that does not exist,",
"		the user's $HOME/.%src file is examined for a",
"		default change field (see aeuconf(5) for more",
"		information).  If that does not exist, when the",
"		user is only working on one change within a",
"		project, that is the default change number.",
"		Otherwise, it is an error.",
"",
"	-DIRectory <path>",
"		This option may be used to specify which directory",
"		is to be used.  It is an error if the current user",
"		does not have appropriate permissions to create",
"		the directory path given.  This must be an",
"		absolute path.",
"",
"		Caution: If you are using an automounter do not",
"		use `pwd` to make an absolute path, it usually",
"		gives the wrong answer.",
"",
"	-Help",
"		This option may be used to obtain more information",
"		about how to use the %s program.",
"",
"	-List",
"		This option may be used to obtain a list of",
"		suitable subjects for this command.  The list may",
"		be more general than expected.",
"",
"	-Project <name>",
"		This option may be used to select the project of",
"		interest.  When no -Project option is specified,",
"		the AEGIS_PROJECT environment variable is",
"		consulted.  If that does not exist, the user's",
"		$HOME/.%src file is examined for a default",
"		project field (see aeuconf(5) for more",
"		information).  If that does not exist, when the",
"		user is only working on changes within a single",
"		project, the project name defaults to that",
"		project.  Otherwise, it is an error.",
"",
"	-TERse",
"		This option may be used to cause listings to",
"		produce the bare minimum of information.  It is",
"		usually useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause %s to produce",
"		more output.  By default %s only produces",
"		output on errors.  When used with the -List option",
"		this option causes column headings to be added.",
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
"	csh%%	alias aedb '%s -db \\!* -v'",
"	sh$	aedb(){%s -db $* -v}",
"",
"ERRORS",
"	It is an error if the change does not exist.",
"	It is an error if the change is not in the awaiting",
"	development state.",
"	It is an error if the current user is not a developer of",
"	the specified project.",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any",
"	error.  The %s command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
	};

	help(text, SIZEOF(text), develop_begin_usage);
}


static void develop_begin_list _((void));

static void
develop_begin_list()
{
	string_ty	*project_name;

	trace(("develop_begin_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_begin_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				develop_begin_usage();
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
		1 << cstate_state_awaiting_development
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void develop_begin_main _((void));

static void
develop_begin_main()
{
	cstate		cstate_data;
	pstate		pstate_data;
	cstate_history	history_data;
	string_ty	*devdir;
	string_ty	*s2;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;

	trace(("develop_begin_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	devdir = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_begin_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				develop_begin_usage();
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				fatal("duplicate -Change option");
			change_number = arglex_value.alv_number;
			if (change_number < 1)
				fatal("change %ld out of range", change_number);
			break;

		case arglex_token_directory:
			if (arglex() != arglex_token_string)
				develop_begin_usage();
			if (devdir)
				fatal("duplicate -DIRectory option");
			/*
			 * To cope with automounters, directories are stored as
			 * given, or are derived from the home directory in the
			 * passwd file.  Within aegis, pathnames have their
			 * symbolic links resolved, and any comparison of paths
			 * is done on this "system idea" of the pathname.
			 */
			devdir = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				develop_begin_usage();
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
		fatal("change number must be stated explicitly");
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	/*
	 * Take an advisory write lock on the appropriate row of the change
	 * table.  Take an advisory write lock on the appropriate row of the
	 * user table.  The user table row may need to be created.
	 * Block while can't get both simultaneously.
	 */
	user_ustate_lock_prepare(up);
	change_cstate_lock_prepare(cp);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * Extract the appropriate row of the change table.
	 * It is an error if the change is not in the
	 * undevelop_begined state.
	 */
	if (cstate_data->state != cstate_state_awaiting_development)
		change_fatal(cp, "not in 'awaiting_development' state");
	if (!project_developer_query(pp, user_name(up)))
	{
		project_fatal
		(
			pp,
			"user \"%S\" is not a developer",
			user_name(up)
		);
	}

	/*
	 * Set the change data to reflect the current user
	 * as developer and move it to the in-development state.
	 * Append another entry to the change history.
	 */
	cstate_data->state = cstate_state_being_developed;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_develop_begin;

	/*
	 * Update user change table to include this change in the list of
	 * changes being developed by this user.
	 */
	user_own_add(up, project_name_get(pp), change_number);

	/*
	 * Create the change directory.
	 */
	if (!devdir)
	{
		s2 = user_default_development_directory(up);
		assert(s2);
		devdir =
			str_format
			(
				"%S/%S.%3.3ld",
				s2,
				project_name_get(pp),
				change_number
			);
		str_free(s2);
	}
	change_development_directory_set(cp, devdir);

	/*
	 * Create the development directory.
	 */
	user_become(up);
	os_mkdir(devdir, 02755);
	undo_rmdir_errok(devdir);
	user_become_undo();
	change_verbose(cp, "development directory \"%S\"", devdir);

	/*
	 * Clear the time fields.
	 */
	cstate_data->build_time = 0;
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;
	cstate_data->regression_test_time = 0;

	/*
	 * Update change table row (and change history table).
	 * Update user table row.
	 * Release advisory write locks.
	 */
	change_cstate_write(cp);
	user_ustate_write(up);
	commit();
	lock_release();

	/*
	 * run the develop begin command
	 */
	change_run_develop_begin_command(cp, up);

	/*
	 * verbose success message
	 */
	change_verbose
	(
		cp,
		"user \"%S\" has begun development",
		user_name(up)
	);
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
develop_begin()
{
	trace(("develop_begin()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		develop_begin_main();
		break;

	case arglex_token_help:
		develop_begin_help();
		break;

	case arglex_token_list:
		develop_begin_list();
		break;
	}
	trace((/*{*/"}\n"));
}
