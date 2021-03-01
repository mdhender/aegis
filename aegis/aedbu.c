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
 * MANIFEST: functions to implement develop begin undo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
	cstate_data->regression_test_time = 0;
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
