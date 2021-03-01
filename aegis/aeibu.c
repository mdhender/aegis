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
 * MANIFEST: functions for implementing integrate begin undo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <aeibu.h>
#include <ael.h>
#include <arglex2.h>
#include <commit.h>
#include <change.h>
#include <dir.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void integrate_begin_undo_usage _((void));

static void
integrate_begin_undo_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -Integrate_Begin_Undo [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_Begin_Undo -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_Begin_Undo -Help\n", progname);
	quit(1);
}


static void integrate_begin_undo_help _((void));

static void
integrate_begin_undo_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Integrate_Begin_Undo - reverse the aeib command",
"",
"SYNOPSIS",
"	%s -Integrate_Begin_Undo [ <option>... ]",
"	%s -Integrate_Begin_Undo -List [ <option>... ]",
"	%s -Integrate_Begin_Undo -Help",
"",
"DESCRIPTION",
"	The %s -Integrate_Begin_Undo command is used to",
"	reverse the actions of the '%s -Integrate_Begin'",
"	command.",
"",
"	Successful execution of this command will move the change",
"	from the 'being_integrated' state to the",
"	'awaiting_integration' state.  The integration directory",
"	will be deleted.  The change will cease to be assigned to",
"	the current user.",
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
"	csh%%	alias aeibu '%s -ibu \\!* -v'",
"	sh$	aeibu(){%s -ibu $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in the",
"	'being_integrated' state.",
"	It is an error if the change is not assigned to the",
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

	help(text, SIZEOF(text), integrate_begin_undo_usage);
}


static void integrate_begin_undo_list _((void (*)(void)));

static void
integrate_begin_undo_list(usage)
	void		(*usage)_((void));
{
	string_ty	*project_name;

	trace(("integrate_begin_undo_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				usage();
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
		1 << cstate_state_being_integrated
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void integrate_begin_undo_main _((void));

static void
integrate_begin_undo_main()
{
	cstate		cstate_data;
	pstate		pstate_data;
	cstate_history	history_data;
	string_ty	*dir;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;

	trace(("integrate_begin_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(integrate_begin_undo_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				integrate_begin_undo_usage();
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
				integrate_begin_undo_usage();
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
	 * lock the change for writing
	 */
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	user_ustate_lock_prepare(up);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * it is an error if the change is not in the 'being_integrated' state.
	 */
	if (cstate_data->state != cstate_state_being_integrated)
		change_fatal(cp, "not in 'being_integrated' state");
	if (!str_equal(change_integrator_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
			"user \"%S\" is not the integrator",
			user_name(up)
		);
	}

	/*
	 * Change the state.
	 * Add to the change's history.
	 */
	cstate_data->state = cstate_state_awaiting_integration;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_integrate_begin_undo;

	/*
	 * remove it from the user's change list
	 */
	user_own_remove(up, project_name_get(pp), change_number);

	/*
	 * Note that the project has no current integration
	 */
	pstate_data->currently_integrating_change = 0;
	dir = str_copy(change_integration_directory_get(cp, 1));
	change_integration_directory_clear(cp);
	cstate_data->build_time = 0;
	cstate_data->delta_number = 0;

	/*
	 * Complain if they are in the integration directory,
	 * because the rmdir at the end can't then run to completion.
	 */
	os_become_orig();
	if (os_below_dir(dir, os_curdir()))
		change_fatal(cp, "please leave the integration directory");
	os_become_undo();

	/*
	 * write out the data and release the locks
	 */
	change_cstate_write(cp);
	user_ustate_write(up);
	project_pstate_write(pp);
	user_become(up);
	commit_rmdir_tree_errok(dir);
	user_become_undo();
	str_free(dir);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	change_verbose(cp, "integrate begin undo");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
integrate_begin_undo()
{
	trace(("integrate_begin_undo()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		integrate_begin_undo_main();
		break;

	case arglex_token_help:
		integrate_begin_undo_help();
		break;

	case arglex_token_list:
		integrate_begin_undo_list(integrate_begin_undo_usage);
		break;
	}
	trace((/*{*/"}\n"));
}
