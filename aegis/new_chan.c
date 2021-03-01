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
 * MANIFEST: functions to create and destroy changes
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <arglex2.h>
#include <cattr.h>
#include <change.h>
#include <col.h>
#include <commit.h>
#include <common.h>
#include <error.h>
#include <help.h>
#include <list.h>
#include <lock.h>
#include <new_chan.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <user.h>


static void new_change_usage _((void));

static void
new_change_usage()
{
	char	*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -New_Change <filename> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Change -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Change -Help\n", progname);
	quit(1);
}


static void new_change_help _((void));

static void
new_change_help()
{
	static char *text[] =
	{
"NAME",
"	%s -New_Change - add a new change to a project",
"",
"SYNOPSIS",
"	%s -New_Change <attr-file> [ <option>... ]",
"	%s -New_Change -List [ <option>... ]",
"	%s -New_Change -Help",
"",
"DESCRIPTION",
"	The %s -New_Change command is used to add a new change",
"	to a project.  See aecattr(5) for information on the",
"	format of the attr-file.",
"",
"	The change is created in the 'awaiting_development'",
"	state.	The change is not assigned to any user.	 The",
"	change has no development directory.",
"",
"	It is not possible to choose your own change number.",
"",
"OPTIONS",
"	The following options are understood:",
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
"	csh%%	alias aenc '%s -nc \\!* -v'",
"	sh$	aenc(){%s -nc $* -v}",
"",
"ERRORS",
"	It is an error if the current user is not an",
"	administrator of the project.  (In some cases it is",
"	possible for developers of a project to create changes,",
"	see aepattr(5) for more information.)",
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

	help(text, SIZEOF(text), new_change_usage);
}


static void new_change_list _((void));

static void
new_change_list()
{
	string_ty	*project_name;

	trace(("new_chane_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_change_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				new_change_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_changes(project_name, 0);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void new_change_main _((void));

static void
new_change_main()
{
	pstate		pstate_data;
	cstate		cstate_data;
	cstate_history	history_data;
	cattr		cattr_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;

	trace(("new_change_main()\n{\n"/*}*/));
	cattr_data = 0;
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_change_usage);
			continue;

		case arglex_token_string:
			if (cattr_data)
				fatal("too many files named");
			os_become_orig();
			cattr_data = cattr_read_file(arglex_value.alv_string);
			os_become_undo();
			if (!cattr_data->brief_description)
			{
				fatal
				(
				  "%s: contains no \"brief_description\" field",
					arglex_value.alv_string
				);
			}
			if (!(cattr_data->mask & cattr_cause_mask))
			{
				fatal
				(
					"%s: contains no \"cause\" field",
					arglex_value.alv_string
				);
			}
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				new_change_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (!cattr_data)
		fatal("no change attributes file named");

	/*
	 * locate project data
	 */
	if (!project_name)
		fatal("project name must be stated explicitly");
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * locate user data
	 */
	up = user_executing(pp);

	/*
	 * Lock the project state file.
	 * Block if necessary.
	 */
	project_pstate_lock_prepare(pp);
	lock_take();
	pstate_data = project_pstate_get(pp);

	/*
	 * it is an error if
	 * the user is not an administrator for the project.
	 */
	if
	(
		!project_administrator_query(pp, user_name(up))
	&&
		(
			!pstate_data->developers_may_create_changes
		||
			!project_developer_query(pp, user_name(up))
		)
	)
	{
		project_fatal
		(
			pp,
			"user \"%S\" is not an administrator",
			user_name(up)
		);
	}

	/*
	 * when developers create changes,
	 * they may not give themselves a testing exemption,
	 * only administrators may do that.
	 */
	if (!project_administrator_query(pp, user_name(up)))
	{
		if
		(
			cattr_data->mask
		&
			(
				cattr_test_exempt_mask
			|
				cattr_test_baseline_exempt_mask
			)
		)
			fatal("developers may not grant testing exemptions");
	}

	/*
	 * Add another row to the change table.
	 * Create the change history.
	 */
	assert(pstate_data->next_change_number >= 1);
	change_number = pstate_data->next_change_number++;
	cp = change_alloc(pp, change_number);
	change_bind_new(cp);

	cstate_data = change_cstate_get(cp);
	cstate_data->state = cstate_state_awaiting_development;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_new_change;
	if (cattr_data->description)
		cstate_data->description = str_copy(cattr_data->description);
	assert(cattr_data->brief_description);
	cstate_data->brief_description = str_copy(cattr_data->brief_description);
	assert(cattr_data->cause);
	cstate_data->cause = cattr_data->cause;
	if (cattr_data->mask & cattr_test_exempt_mask)
		cstate_data->test_exempt = cattr_data->test_exempt;
	else
		cstate_data->test_exempt = pstate_data->default_test_exemption;
	if (change_number == 1)
		cstate_data->test_baseline_exempt = 1;
	else
	{
		if (cattr_data->mask & cattr_test_exempt_mask)
			cstate_data->test_baseline_exempt =
				cattr_data->test_baseline_exempt;
		else
			cstate_data->test_baseline_exempt =
				pstate_data->default_test_exemption;
	}
	cattr_type.free(cattr_data);

	/*
	 * Write out the change file.
	 * There is no need to lock this file
	 * as it does not exist yet;
	 * the project state file, with the number in it, is locked.
	 */
	change_cstate_write(cp);

	/*
	 * Add the change to the list of existing changes.
	 * Incriment the next_change_number.
	 * and write pstate back out.
	 */
	project_change_append(pp, change_number);

	/*
	 * Unlock the pstate file.
	 */
	project_pstate_write(pp);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	change_verbose(cp, "created");
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
new_change()
{
	trace(("new_change()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		new_change_main();
		break;

	case arglex_token_help:
		new_change_help();
		break;

	case arglex_token_list:
		new_change_list();
		break;
	}
	trace((/*{*/"}\n"));
}


static void new_change_undo_usage _((void));

static void
new_change_undo_usage()
{
	char	*progname;

	progname = option_get_progname();
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
"NAME",
"	%s -New_Change_Undo - remove a new change from a project",
"",
"SYNOPSIS",
"	%s -New_Change_Undo [ <option>... ]",
"	%s -New_Change_Undo -List [ <option>... ]",
"	%s -New_Change_Undo -Help",
"",
"DESCRIPTION",
"	The %s -New_Change_Undo command is used to remove a",
"	new change from a project.",
"",
"	It wan't called '%s -Remove_Change' in order to",
"	emphasize that fact the the change must be in the",
"	'awaiting_development' state.  In practice it is",
"	possible, with a combination of commands, to remove any",
"	change which has not reached the 'completed' state.",
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
"	csh%%	alias aencu '%s -ncu \\!* -v'",
"	sh$	aencu(){%s -ncu $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in the",
"	'awaiting_development' state.",
"	It is an error if the current user is not an",
"	administrator of the project.",
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
		change_fatal(cp, "not in 'awaiting_development' state");
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
