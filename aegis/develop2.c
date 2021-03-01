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
 * MANIFEST: functions to enter and leave 'being_developed' state
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <arglex2.h>
#include <change.h>
#include <col.h>
#include <commit.h>
#include <common.h>
#include <develop2.h>
#include <dir.h>
#include <error.h>
#include <help.h>
#include <list.h>
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

	progname = option_get_progname();
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


static void develop_begin_undo_usage _((void));

static void
develop_begin_undo_usage()
{
	char		*progname;

	progname = option_get_progname();
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
"NAME",
"	%s -Develop_Begin_Undo - undo the effects of aedb",
"",
"SYNOPSIS",
"	%s -Develop_Begin_Undo <change-number> [ <option>... ]",
"	%s -Develop_Begin_Undo -List [ <option>... ]",
"	%s -Develop_Begin_Undo -Help",
"",
"DESCRIPTION",
"	The %s -Develop_Begin_Undo command is used to reverse",
"	the effects of the '%s -Develop_Begin' command.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Change <number>",
"		This option may be used to specify a particular",
"		change within a project.  When no -Change option is",
"		specified, the AEGIS_CHANGE environment variable is",
"		consulted.  If that does not exist, the user's",
"		$HOME/.aegisrc file is examined for a default change",
"		field (see aeuconf(5) for more information).  If",
"		that does not exist, when the user is only working",
"		on one change within a project, that is the default",
"		change number.  Otherwise, it is an error.",
"",
"	-Help",
"		This option may be used to obtain more",
"		information about how to use the %s program.",
"",
"	-Keep",
"		This option may be used to retain files and/or",
"		directories usually deleted by the command.",
"",
"	-List",
"		This option may be used to obtain a list of",
"		suitable subjects for this command.  The list may",
"		be more general than expected.",
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
"		This option may be used to cause listings to",
"		produce the bare minimum of information.  It is",
"		usually useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause %s to produce",
"		more output.  By default %s only produces",
"		output on errors.  When used with the -List",
"		option this option causes column headings to be",
"		added.",
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
"	csh%%	alias aedbu '%s -dbu \\!* -v'",
"	sh$	aedbu(){%s -dbu $* -v}",
"",
"ERRORS",
"	It is an error if the change is no assigned to the",
"	current user.",
"	It is an error if the change is not in the",
"	'being_developed' state.",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any",
"	error.	The %s command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
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
	int		keep;
	project_ty	*pp;
	user_ty		*up;
	change_ty	*cp;
	pstate		pstate_data;
	cstate		cstate_data;
	cstate_history	history_data;

	trace(("develop_begin_undo_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	keep = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_begin_undo_usage);
			continue;

		case arglex_token_keep:
			if (keep)
			{
				fatal
				(
					"duplicate %s option",
					arglex_value.alv_string
				);
			}
			keep++;
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
		change_fatal(cp, "not in 'being_developed' state");
	if (!str_equal(change_developer_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
			"user \"%S\" is not the developer",
			user_name(up)
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
	cstate_data->build_time = 0;
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;
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
	if (!keep)
	{
		string_ty	*dd;

		change_verbose(cp, "remove development directory");
		dd = change_development_directory_get(cp, 1);
		change_become(cp);
		commit_rmdir_tree_errok(dd);
		change_become_undo();
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


static void develop_end_usage _((void));

static void
develop_end_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -Develop_End [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Develop_End -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Develop_End -Help\n", progname);
	quit(1);
}


static void develop_end_help _((void));

static void
develop_end_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Develop_End - complete development of a change",
"",
"SYNOPSIS",
"	%s -Develop_End [ <option>... ]",
"	%s -Develop_End -List [ <option>... ]",
"	%s -Develop_End -Help",
"",
"DESCRIPTION",
"	The %s -Develop_End command is used to notify",
"	%s of the completion of the development of a change.",
"",
"	Successful execution of the command advances the change",
"	from the 'being_developed' state to the 'being_reviewed'",
"	state.",
"",
"	The ownership of files in the development directory is",
"	changed to the project owner and group, and the files",
"	changed to be read-only.  This prevents accidental",
"	alterations of the change's files between development and",
"	integration.",
"",
"	The change is no longer considered assigned to the",
"	current user.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Change <number>",
"		This option may be used to specify a particular",
"		change within a project.  When no -Change option is",
"		specified, the AEGIS_CHANGE environment variable is",
"		consulted.  If that does not exist, the user's",
"		$HOME/.aegisrc file is examined for a default change",
"		field (see aeuconf(5) for more information).  If",
"		that does not exist, when the user is only working",
"		on one change within a project, that is the default",
"		change number.  Otherwise, it is an error.",
"",
"	-Help",
"		This option may be used to obtain more",
"		information about how to use the %s program.",
"",
"	-List",
"		This option may be used to obtain a list of",
"		suitable subjects for this command.  The list may",
"		be more general than expected.",
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
"		This option may be used to cause listings to",
"		produce the bare minimum of information.  It is",
"		usually useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause %s to produce",
"		more output.  By default %s only produces",
"		output on errors.  When used with the -List",
"		option this option causes column headings to be",
"		added.",
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
"	csh%%	alias aede '%s -de \\!* -v'",
"	sh$	aede(){%s -de $* -v}",
"",
"ERRORS",
"	It is an error if the change is not assigned to the",
"	current user.",
"	It is an error if The change is not in the",
"	'being_developed' state.",
"	It is an error if there has been no successful '%s",
"	-Build' command since a change file was last edited.",
"	It is an error if there has been no successful '%s",
"	-DIFFerence' command since a change file was last edited.",
"	It is an error if there has been no successful '%s",
"	-Test' command since a change file was last edited.",
"	It is an error if there has been no successful '%s",
"	-Test -BaseLine' command since a change file was last",
"	edited.",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any",
"	error.	The %s command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
	};

	help(text, SIZEOF(text), develop_end_usage);
}


static void develop_end_list _((void));

static void
develop_end_list()
{
	string_ty	*project_name;

	trace(("develop_end_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_end_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				develop_end_usage();
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


static void repair_diff_time _((change_ty *, string_ty *));

static void
repair_diff_time(cp, path)
	change_ty	*cp;
	string_ty	*path;
{
	string_ty	*s;
	cstate_src	src_data;
	string_ty	*s2;

	s = os_below_dir(change_development_directory_get(cp, 1), path);
	src_data = change_src_find(cp, s);
	if (src_data)
		src_data->diff_time = os_mtime(path);
	else
	{
		if
		(
			s->str_length > 2
		&&
			!strcmp(s->str_text + s->str_length - 2, ",D")
		)
		{
			s2 = str_n_from_c(s->str_text, s->str_length - 2);
			src_data = change_src_find(cp, s2);
			if (src_data)
				src_data->diff_file_time = os_mtime(path);
			str_free(s2);
		}
	}
	str_free(s);
}


static void de_func _((void *, dir_walk_message_ty, string_ty *, struct stat *));

static void
de_func(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	change_ty	*cp;
	int		uid;

	trace(("de_func(message = %d, path = \"%s\", st = %08lX)\n{\n"/*}*/,
		message, path->str_text, st));
	cp = (change_ty *)arg;
	switch (message)
	{
	case dir_walk_file:
	case dir_walk_dir_after:
		os_become_query(&uid, (int *)0, (int *)0);
		if (st->st_uid == uid)
		{
			os_chmod(path, st->st_mode & 07555);
			undo_chmod(path, st->st_mode);
		}
		repair_diff_time(cp, path);
		break;

	case dir_walk_special:
	case dir_walk_symlink:
	case dir_walk_dir_before:
		break;
	}
	trace((/*{*/"}\n"));
}


static void develop_end_main _((void));

static void
develop_end_main()
{
	string_ty	*dd;
	cstate		cstate_data;
	pstate		pstate_data;
	int		j;
	cstate_history	history_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	int		build_whine;
	int		test_whine;
	int		test_bl_whine;
	int		diff_whine;
	int		errs;

	trace(("develop_end_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_end_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				develop_end_usage();
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
				develop_end_usage();
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
	 * It is an error if the change is not in the in-development state.
	 * It is an error if the change is not assigned to the current user.
	 * It is an error if the change has no current diff.
	 * It is an error if the change has no current build.
	 * It is an error if the change has no current test pass.
	 * It is an error if the change has no current baseline test pass.
	 * It is an error if the change has no new test associtaed with it.
	 */
	if (cstate_data->state != cstate_state_being_developed)
		change_fatal(cp, "not in 'being_developed' state");
	if (!str_equal(change_developer_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
			"user \"%S\" is not the developer",
			user_name(up)
		);
	}
	if (!cstate_data->src->length)
		change_fatal(cp, "no files");
	errs = 0;
	build_whine = 0;
	test_whine = 0;
	test_bl_whine = 0;
	diff_whine = 0;

	/*
	 * It is an error if any files in the change file table have been
	 * modified since the last build.
	 * It is an error if any files in the change file table have been
	 * modified since the last diff.
	 */
	dd = change_development_directory_get(cp, 1);
	user_become(up);
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		cstate_src	c_src_data;
		pstate_src	p_src_data;
		string_ty	*path;
		string_ty	*path_d;
		long		when;
		long		when_d;

		c_src_data = cstate_data->src->list[j];
		if (c_src_data->action == file_action_remove)
			continue;
		path =
			str_format("%S/%S", dd, c_src_data->file_name);
		if (!os_exists(path))
		{
			change_error
			(
				cp,
				"file \"%s\" not found",
				c_src_data->file_name->str_text
			);
			str_free(path);
			errs++;
			continue;
		}
		path_d = str_format("%S,D", path);
		when = os_mtime(path);
		str_free(path);
		if (os_exists(path_d))
			when_d = os_mtime(path_d);
		else
			when_d = 0;
		str_free(path_d);
		if
		(
			when >= cstate_data->build_time
		||
			!cstate_data->build_time
		)
		{
			if (!build_whine)
			change_error
			(
				cp,
				"no current '%s -Build' registration",
				option_get_progname()
			);
			build_whine++;
			errs++;
		}
		if
		(
			when != c_src_data->diff_time
		||
			!c_src_data->diff_time
		||
			when_d != c_src_data->diff_file_time
		||
			!c_src_data->diff_file_time
		)
		{
			if (!diff_whine)
			change_error
			(
				cp,
				"no current '%s -Diff' registration",
				option_get_progname()
			);
			diff_whine++;
			errs++;
		}
		if
		(
			!cstate_data->test_exempt
		&&
			(
				when >= cstate_data->test_time
			||
				!cstate_data->test_time
			)
		)
		{
			if (!test_whine)
			change_error
			(
				cp,
				"no current '%s -Test' registration",
				option_get_progname()
			);
			test_whine++;
			errs++;
		}
		if
		(
			!cstate_data->test_baseline_exempt
		&&
			(
				when >= cstate_data->test_baseline_time
			||
				!cstate_data->test_baseline_time
			)
		)
		{
			if (!test_bl_whine)
			change_error
			(
				cp,
				"no current '%s -Test -BaseLine' registration",
				option_get_progname()
			);
			test_bl_whine++;
			errs++;
		}

		/*
		 * It is an error if any files in the change file table haved
		 * different edit numbers to the baseline file table edit
		 * numbers.
		 */
		if (c_src_data->action != file_action_create)
		{
			p_src_data = project_src_find(pp, c_src_data->file_name);
			if (!p_src_data)
			{
				change_error
				(
					cp,
					"file \"%S\" no longer in baseline",
					c_src_data->file_name
				);
				errs++;
			}
			if
			(
				!
					str_equal
					(
						c_src_data->edit_number,
						p_src_data->edit_number
					)
			)
			{
				change_error
				(
					cp,
      "file \"%S\" in baseline has changed since last '%s -DIFFerence' command",
					c_src_data->file_name,
					option_get_progname()
				);
				errs++;
			}

			/*
			 * make sure we can lock the file
			 */
			if (p_src_data->locked_by)
			{
				change_error
				(
					cp,
					"file \"%S\" locked for change %d",
					c_src_data->file_name,
					p_src_data->locked_by
				);
				errs++;
			}
		}
		else
		{
			/*
			 * add a new entry to the pstate src list,
			 * and mark it as "about to be created".
			 */
			p_src_data = project_src_new(pp, c_src_data->file_name);
			p_src_data->usage = c_src_data->usage;
			p_src_data->about_to_be_created_by = change_number;
		}
		p_src_data->locked_by = change_number;
	}
	if (errs)
		quit(1);

	/*
	 * Change all of the files in the development directory
	 * to be read-only, and record the new ctime.
	 */
	dir_walk(dd, de_func, cp);
	user_become_undo();

	/*
	 * add to history for state change
	 */
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_develop_end;

	/*
	 * Advance the change to the being-reviewed state.
	 * Clear the build-time field.
	 * Clear the test-time field.
	 * Clear the test-baseline-time field.
	 */
	cstate_data->state = cstate_state_being_reviewed;
	cstate_data->build_time = 0;
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;

	/*
	 * Remove the change from the list of assigned changes in the user
	 * change table (in the user row).
	 */
	user_own_remove(up, project_name_get(pp), change_number);

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
	 * run the notify command
	 */
	change_run_develop_end_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, "development completed");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
develop_end()
{
	trace(("develop_end()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		develop_end_main();
		break;

	case arglex_token_help:
		develop_end_help();
		break;

	case arglex_token_list:
		develop_end_list();
		break;
	}
	trace((/*{*/"}\n"));
}


static void develop_end_undo_usage _((void));

static void
develop_end_undo_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -Undo_Develop_End [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Undo_Develop_End -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Undo_Develop_End -Help\n", progname);
	quit(1);
}


static void develop_end_undo_help _((void));

static void
develop_end_undo_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Develop_End_Undo - recall a change for further",
"	development",
"",
"SYNOPSIS",
"	%s -Develop_End_Undo <change-number> [ <option>... ]",
"	%s -Develop_End_Undo -List [ <option>... ]",
"	%s -Develop_End_Undo -Help",
"",
"DESCRIPTION",
"	The %s -Develop_End_Undo command is used to recall a",
"	change for further development.",
"",
"	Successful execution of this command returns the change",
"	to the 'being_developed' state.",
"",
"	The files are changed back to being owned by the current",
"	user, and cease to be read-only.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Change <number>",
"		This option may be used to specify a particular",
"		change within a project.  When no -Change option is",
"		specified, the AEGIS_CHANGE environment variable is",
"		consulted.  If that does not exist, the user's",
"		$HOME/.aegisrc file is examined for a default change",
"		field (see aeuconf(5) for more information).  If",
"		that does not exist, when the user is only working",
"		on one change within a project, that is the default",
"		change number.  Otherwise, it is an error.",
"",
"	-Help",
"		This option may be used to obtain more",
"		information about how to use the %s program.",
"",
"	-List",
"		This option may be used to obtain a list of",
"		suitable subjects for this command.  The list may",
"		be more general than expected.",
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
"		This option may be used to cause listings to",
"		produce the bare minimum of information.  It is",
"		usually useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause %s to produce",
"		more output.  By default %s only produces",
"		output on errors.  When used with the -List",
"		option this option causes column headings to be",
"		added.",
"",
"	All options are case insensitive.  Options may be",
"	abbreviated; the abbreviation is the upper case letters.",
"	Options and other command line arguments may be mixed",
"	arbitrarily on the command line.",
"",
"RECOMMENDED ALIAS",
"	The recommended alias for this command is",
"	csh%%	alias aedeu '%s -deu \\!* -v'",
"	sh$	aedeu(){%s -deu $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in one of the",
"	'being_reviewed' or 'awaiting_integration' states.",
"	It is an error if the change was not developed by the",
"	current user.",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any",
"	error.	The %s command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
	};

	help(text, SIZEOF(text), develop_end_undo_usage);
}


static void develop_end_undo_list _((void));

static void
develop_end_undo_list()
{
	string_ty	*project_name;

	trace(("develop_end_undo_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_end_undo_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				develop_end_undo_usage();
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
		(
			(1 << cstate_state_being_reviewed)
		|
			(1 << cstate_state_awaiting_integration)
		)
	);
	trace((/*{*/"}\n"));
}


static void deu_func _((void *, dir_walk_message_ty, string_ty *, struct stat *));

static void
deu_func(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	change_ty	*cp;
	int		uid;

	trace(("deu_func(message = %d, path = \"%s\", st = %08lX)\n{\n"/*}*/,
		message, path->str_text, st));
	cp = (change_ty *)arg;
	switch (message)
	{
	case dir_walk_dir_before:
	case dir_walk_file:
		os_become_query(&uid, (int *)0, (int *)0);
		if (st->st_uid == uid)
		{
			os_chmod(path, st->st_mode | 0200);
			undo_chmod(path, st->st_mode);
		}
		repair_diff_time(cp, path);
		break;

	case dir_walk_special:
	case dir_walk_symlink:
	case dir_walk_dir_after:
		break;
	}
	trace((/*{*/"}\n"));
}


static void develop_end_undo_main _((void));

static void
develop_end_undo_main()
{
	cstate		cstate_data;
	pstate		pstate_data;
	cstate_history	history_data;
	int		j;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	string_ty	*dd;

	trace(("develop_end_undo_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_end_undo_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				develop_end_undo_usage();
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
				develop_end_undo_usage();
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
		change_number = user_default_change(up);;
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	/*
	 * lock the change for writing
	 */
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	user_ustate_lock_prepare(up);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * It is an error if the change is not in one of the 'being_reviewed'
	 * or 'awaiting_integration' states.
	 * It is an error if the current user did not develop the change.
	 */
	if
	(
		cstate_data->state != cstate_state_being_reviewed
	&&
		cstate_data->state != cstate_state_awaiting_integration
	)
		change_fatal(cp, "not in 'being_reviewed' state");
	if (!str_equal(change_developer_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
			"user \"%S\" was not the developer",
			user_name(up)
		);
	}

	/*
	 * Change the state.
	 * Add to the change's history.
	 */
	cstate_data->state = cstate_state_being_developed;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_develop_end_undo;
	cstate_data->build_time = 0;

	/*
	 * add it back into the user's change list
	 */
	user_own_add(up, project_name_get(pp), change_number);

	/*
	 * go through the files in the change and unlock them
	 * in the baseline
	 */
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		cstate_src	c_src_data;
		pstate_src	p_src_data;

		c_src_data = cstate_data->src->list[j];
		p_src_data = project_src_find(pp, c_src_data->file_name);
		if (!p_src_data)
			/* this is really a corrupted file */
			continue;
		p_src_data->locked_by = 0;

		/*
		 * Remove the file if it is about_to_be_created
		 * by the change we are rescinding.
		 */
		if (p_src_data->about_to_be_created_by)
		{
			assert(p_src_data->about_to_be_created_by == change_number);
			project_src_remove(pp, c_src_data->file_name);
		}
	}

	/*
	 * change the ownership back to the user
	 */
	dd = change_development_directory_get(cp, 1);
	user_become(up);
	dir_walk(dd, deu_func, cp);
	user_become_undo();

	/*
	 * write out the data and release the locks
	 */
	change_cstate_write(cp);
	project_pstate_write(pp);
	user_ustate_write(up);
	commit();
	lock_release();

	/*
	 * run the notify command
	 */
	change_run_develop_end_undo_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, "development resumed");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
develop_end_undo()
{
	trace(("develop_end_undo()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		develop_end_undo_main();
		break;

	case arglex_token_help:
		develop_end_undo_help();
		break;

	case arglex_token_list:
		develop_end_undo_list();
		break;
	}
	trace((/*{*/"}\n"));
}
