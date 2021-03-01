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
 * MANIFEST: functions to list and modify change attributes
 */

#include <stdio.h>
#include <stdlib.h>

#include <arglex2.h>
#include <cattr.h>
#include <cattr_ed.h>
#include <commit.h>
#include <change.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <project.h>
#include <option.h>
#include <os.h>
#include <trace.h>
#include <user.h>


static void change_attributes_usage _((void));

static void
change_attributes_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -Change_Attributes [ <option>... ] <attr-file>\n", progname);
	fprintf(stderr, "       %s -Change_Attributes -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Change_Attributes -Help\n", progname);
	quit(1);
}


static void change_attributes_help _((void));

static void
change_attributes_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Change_Attributes - modify the attributes of a",
"	change",
"",
"SYNOPSIS",
"	%s -Change_Attributes [ <option>... ] <attr-file>",
"	%s -Change_Attributes -List [ <option>... ]",
"	%s -Change_Attributes -Help",
"",
"DESCRIPTION",
"	The %s -Change_Attributes command is used to modify",
"	the attributes of a change.  The best method is to use",
"	the -List option to get a copy of the current attributes",
"	(redirect it into a file), edit those attributes as you",
"	want them, and then feed it back as the attr-file.  See",
"	aecattr(5) for more information.",
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
"	csh%%	alias aeca '%s -ca \\!* -v'",
"	sh$	aeca(){%s -ca $* -v}",
"",
"ERRORS",
"	It is an error if the current user is not an",
"	administrator of the specified project.",
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

	help(text, SIZEOF(text), change_attributes_usage);
}


static void change_attributes_list _((void));

static void
change_attributes_list()
{
	string_ty	*project_name;
	project_ty	*pp;
	cattr		cattr_data;
	cstate		cstate_data;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;

	trace(("change_attributes_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(change_attributes_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				change_attributes_usage();
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
				change_attributes_usage();
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
	 * build the cattr data
	 */
	cstate_data = change_cstate_get(cp);
	cattr_data = (cattr)cattr_type.alloc();
	if (cstate_data->description)
		cattr_data->description = str_copy(cstate_data->description);
	if (cstate_data->brief_description)
		cattr_data->brief_description =
			str_copy(cstate_data->brief_description);
	cattr_data->cause = cstate_data->cause;
	cattr_data->test_exempt = cstate_data->test_exempt;
	cattr_data->test_baseline_exempt = cstate_data->test_baseline_exempt;

	/*
	 * print the cattr data
	 */
	cattr_write_file((char *)0, cattr_data);
	cattr_type.free(cattr_data);
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


static void change_attributes_main _((void));

static void
change_attributes_main()
{
	string_ty	*project_name;
	project_ty	*pp;
	cattr		cattr_data = 0;
	cstate		cstate_data;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;

	trace(("change_attributes_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(change_attributes_usage);
			continue;

		case arglex_token_string:
			if (cattr_data)
				fatal("too many files named");
			os_become_orig();
			cattr_data = cattr_read_file(arglex_value.alv_string);
			os_become_undo();
			assert(cattr_data);
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				change_attributes_usage();
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
				change_attributes_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (!cattr_data)
		fatal("no file named");

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
	 * lock the change
	 */
	change_cstate_lock_prepare(cp);
	lock_take();
	cstate_data = change_cstate_get(cp);

	/*
	 * It is an error if the change is not in the "being developed" state.
	 * It is an error if the user is not an administrator or the developer.
	 */
	if
	(
		!project_administrator_query(pp, user_name(up))
	&&
		(
			cstate_data->state != cstate_state_being_developed
		||
			!str_equal(change_developer_name(cp), user_name(up))
		)
	)
	{
		change_fatal
		(
			cp,
			"attributes may only be changed by a project \
administrator, or by the developer during development"
		);
	}

	/*
	 * copy the attributes across
	 */
	if (cattr_data->description)
	{
		if (cstate_data->description)
			str_free(cstate_data->description);
		cstate_data->description = str_copy(cattr_data->description);
	}
	if (cattr_data->brief_description)
	{
		if (cstate_data->brief_description)
			str_free(cstate_data->brief_description);
		cstate_data->brief_description =
			str_copy(cattr_data->brief_description);
	}
	if (cattr_data->mask & cattr_cause_mask)
		cstate_data->cause = cattr_data->cause;
	if (project_administrator_query(pp, user_name(up)))
	{
		if (cattr_data->mask & cattr_test_exempt_mask)
			cstate_data->test_exempt = cattr_data->test_exempt;
		if (cattr_data->mask & cattr_test_baseline_exempt_mask)
			cstate_data->test_baseline_exempt =
				cattr_data->test_baseline_exempt;
	}
	else
	{
		if
		(
			(
				cattr_data->test_exempt
			&&
				!cstate_data->test_exempt
			)
		||
			(
				cattr_data->test_baseline_exempt
			&&
				!cstate_data->test_baseline_exempt
			)
		)
		{
			change_fatal
			(
				cp,
		   "only project administrators may exempt changes from testing"
			);
		}
		else
		{
			/*
			 * developers may remove exemptions
			 */
			if
			(
				(cattr_data->mask & cattr_test_exempt_mask)
			&&
				!cattr_data->test_exempt
			)
				cstate_data->test_exempt = 0;
			if
			(
				(cattr_data->mask & cattr_test_baseline_exempt_mask)
			&&
				!cattr_data->test_baseline_exempt
			)
				cstate_data->test_baseline_exempt = 0;
		}
	}

	cattr_type.free(cattr_data);
	change_cstate_write(cp);
	commit();
	lock_release();
	change_verbose(cp, "attributes changed");
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
change_attributes()
{
	trace(("change_attributes()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		change_attributes_main();
		break;

	case arglex_token_help:
		change_attributes_help();
		break;

	case arglex_token_list:
		change_attributes_list();
		break;
	}
	trace((/*{*/"}\n"));
}
