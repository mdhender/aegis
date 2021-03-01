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
 * MANIFEST: functions to perform development and integration builds
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include <aeb.h>
#include <ael.h>
#include <arglex2.h>
#include <col.h>
#include <commit.h>
#include <change.h>
#include <error.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


/*
 * NAME
 *	build_usage
 *
 * SYNOPSIS
 *	void build_usage(void);
 *
 * DESCRIPTION
 *	The build_usage function is used to
 *	briefly describe how to used the 'aegis -Build' command.
 */

static void build_usage _((void));

static void
build_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -Build [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Build -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Build -Help\n", progname);
	quit(1);
}


/*
 * NAME
 *	build_help
 *
 * SYNOPSIS
 *	void build_help(void);
 *
 * DESCRIPTION
 *	The build_help function is used to
 *	describe in detail how to use the 'aegis -Build' command.
 */

static void build_help _((void));

static void
build_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Build - build a project",
"",
"SYNOPSIS",
"	%s -Build [ option... ]",
"	%s -Build -List [ option... ]",
"	%s -Build -Help",
"",
"DESCRIPTION",
"	The %s -Build command is used to build a project.  The",
"	project configuration file is consulted for the",
"	appropriate build command, and that command is executed",
"	(see aepconf(5) for more information.)  Output of the",
"	command is automatically logged to the %s.log file at",
"	the root of the development directory tree.  The build",
"	command will be executed with its current directory being",
"	the root of the development directory, irrespective of",
"	there the %s -Build command was exectuted.",
"",
"	If the change is in the 'being_integrated' state,",
"	references to the development directory, above, should be",
"	read as the integration directory.  Integration build",
"	commands are executed with the user and group set to the",
"	project's owning user and group.  That is, it is not",
"	necessary for an integrator to log in as someone else,",
"	the project account for instance, in order to do an",
"	integration.",
"",
"	While there is a build in progress for any change in a",
"	project, an integrate pass for the project will wait until",
"	all the builds are completed before starting.  This is to",
"	ensure that the baseline is consistent for the entire build.",
"	Similarly, while an integrate pass is in progress for a",
"	project, any builds will wait until it is completed before",
"	starting.",
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
"RECOMMENDED ALIAS",
"	The recommended alias for this command is",
"	csh%%	alias aeb '%s -b \\!* -v'",
"	sh$	aeb(){%s -b $* -v}",
"",
"ERRORS",
"	It is an error if the change is not assigned to the",
"	curent user.",
"	It is an error if the change is not in one of the",
"	'being_developed' or 'being_integrated' states.",
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

	help(text, SIZEOF(text), build_usage);
}


/*
 * NAME
 *	build_list
 *
 * SYNOPSIS
 *	void build_list(void);
 *
 * DESCRIPTION
 *	The build_list function is used to
 *	list the changes which may be built within the project.
 */

static void build_list _((void));

static void
build_list()
{
	string_ty	*project_name;

	trace(("build_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(build_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				build_usage();
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
			(1 << cstate_state_being_integrated)
		)
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	build_main
 *
 * SYNOPSIS
 *	void build_main(void);
 *
 * DESCRIPTION
 *	The build_main function is used to
 *	build a change in the "being developed" or "being integrated" states.
 *	It extracts what to do from the command line.
 */

static void build_main _((void));

static void
build_main()
{
	string_ty	*the_command;
	cstate		cstate_data;
	pstate		pstate_data;
	pconf		pconf_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;

	trace(("build_main()\n{\n"/*}*/));
	nolog = 0;
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(build_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				build_usage();
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
				build_usage();
			/* fall through... */

		case arglex_token_string:
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (nolog)
				fatal("duplicate %s option", arglex_value.alv_string);
			nolog = 1;
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
	 * Take an advisory write lock on this row of the change table.
	 * Block if necessary.
	 */
	change_cstate_lock_prepare(cp);
	project_build_read_lock_prepare(pp);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * Extract the appropriate row of the change table.
	 * It is an error if the change is not in the in-development state.
	 * It is an error if the change is not assigned to the current user.
	 * It is an error if the change has no files assigned.
	 */
	switch (cstate_data->state)
	{
	default:
		change_fatal(cp, "not in 'being_developed' state");
		break;

	case cstate_state_being_developed:
		if (!str_equal(change_developer_name(cp), user_name(up)))
		{
			change_fatal
			(
				cp,
				"user \"%S\" is not the developer",
				user_name(up)
			);
		}
		assert(cstate_data->src);
		if (!cstate_data->src->length)
			change_fatal(cp, "has no files");
		break;

	case cstate_state_being_integrated:
		if (!str_equal(change_integrator_name(cp), user_name(up)))
		{
			change_fatal
			(
				cp,
				"user \"%S\" is not the integrator",
				user_name(up)
			);
		}
		break;
	}

	/*
	 * Update the time the build was done.
	 * This will not be written out if the build fails.
	 */
	time(&cstate_data->build_time);

	/*
	 * get the command to execute
	 *  1. if the change is editing config, use that
	 *  2. if the baseline contains config, use that
	 *  3. error if can't find one (DON'T look for file existence)
	 */
	pconf_data = change_pconf_get(cp);
	if
	(
		!pconf_data->development_build_command
	||
		!pconf_data->build_command
	)
	{
		change_fatal
		(
			cp,
		    "you need to create a new \"%s\" file before you can build",
			THE_CONFIG_FILE
		);
	}

	/*
	 * work out the build command
	 */
	trace(("work out the build command\n"));
	if (cstate_data->state == cstate_state_being_integrated)
	{
		/*
		 * %1 = project name
		 * %2 = change number
		 * %3 = identifying string, in the form "a.b.Dnnn"
		 *	where 'a' is the major version number,
		 *	'b' is the minor version number,
		 *	and 'nnn' is the build number.
		 */
		sub_var_set("1", "${project}");
		sub_var_set("2", "${change}");
		sub_var_set("3", "${version}");
		the_command = pconf_data->build_command;
		the_command = substitute(cp, the_command);
	}
	else
	{
		/*
		 * %1 = project name
		 * %2 = change number
		 * %3 = identifying string, in the form "a.b.Cnnn"
		 *	where 'a' is the major version number,
		 *	'b' is the minor version number,
		 *	and 'nnn' is the change number.
		 * %4 = the absolute path of the project baseline directory.
		 */
		sub_var_set("1", "${project}");
		sub_var_set("2", "${change}");
		sub_var_set("3", "${version}");
		sub_var_set("4", "${baseline}");
		the_command = pconf_data->development_build_command;
		the_command = substitute(cp, the_command);
	}

	/*
	 * the program has changed, so it needs testing again,
	 * so stomp on the validation fields.
	 */
	trace(("nuke time stamps\n"));
	cstate_data->test_time = 0;
	cstate_data->regression_test_time = 0;
	cstate_data->test_baseline_time = 0;

	/*
	 * do the build
	 */
	trace(("open the log file\n"));
	trace(("do the build\n"));
	if (cstate_data->state == cstate_state_being_integrated)
	{
		string_ty	*id;

		if (!nolog)
		{
			user_ty		*pup;

			pup = project_user(pp);
			log_open(change_logfile_get(cp), pup);
			user_free(pup);
		}
		change_verbose(cp, "integration build started");
		id = change_integration_directory_get(cp, 0);
		project_become(pp);
		os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, id);
		project_become_undo();
		change_verbose(cp, "integration build complete");
	}
	else
	{
		string_ty	*dd;

		if (!nolog)
			log_open(change_logfile_get(cp), up);
		change_verbose(cp, "development build started");
		change_run_project_file_command(cp);
		dd = change_development_directory_get(cp, 0);
		user_become(up);
		os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dd);
		user_become_undo();
		change_verbose(cp, "development build complete");
	}

	/*
	 * Update change data with result of build.
	 * (This will be used when validating developer sign off.)
	 * Release advisory write lock on row of change table.
	 */
	change_cstate_write(cp);
	commit();
	lock_release();
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	build
 *
 * SYNOPSIS
 *	void build(void);
 *
 * DESCRIPTION
 *	The build function is used to
 *	dispatch the 'aegis -Build' command to the relevant functionality.
 *	Where it goes depends on the command line.
 */

void
build()
{
	trace(("build()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		build_main();
		break;

	case arglex_token_help:
		build_help();
		break;

	case arglex_token_list:
		build_list();
		break;
	}
	trace((/*{*/"}\n"));
}
