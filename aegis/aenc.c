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
 * MANIFEST: functions to implement new change
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <aeca.h>
#include <aenc.h>
#include <ael.h>
#include <arglex2.h>
#include <cattr.h>
#include <change.h>
#include <col.h>
#include <commit.h>
#include <common.h>
#include <error.h>
#include <help.h>
#include <lock.h>
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

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -New_Change <attr-file> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Change -Edit [ <option>... ]\n", progname);
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
"	%s -New_Change -Edit [ <option>... ]",
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
"	-Edit",
"		Edit the attributes with a text editor, this is",
"		usually more convenient than supplying a text",
"		file.  The EDITOR environment variable will be",
"		consulted for the name of the editor to use;",
"		defaults to vi(1) if not set.  Warning: not well",
"		behaved when faced with errors, the temporary",
"		file is always deleted.",
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


static void cattr_defaults _((cattr, pstate));

static void
cattr_defaults(a, ps)
	cattr		a;
	pstate		ps;
{
	if
	(
		a->cause == change_cause_internal_improvement
	||
		a->cause == change_cause_internal_improvement
	)
	{
		if (!(a->mask & cattr_test_exempt_mask))
		{
			a->test_exempt = 1;
			a->mask |= cattr_test_exempt_mask;
		}
		if (!(a->mask & cattr_test_baseline_exempt_mask))
		{
			a->test_baseline_exempt = 1;
			a->mask |= cattr_test_baseline_exempt_mask;
		}
		if (!(a->mask & cattr_regression_test_exempt_mask))
		{
			a->regression_test_exempt = 0;
			a->mask |= cattr_regression_test_exempt_mask;
		}
	}
	else
	{
		if (!(a->mask & cattr_regression_test_exempt_mask))
		{
			a->regression_test_exempt = 1;
			a->mask |= cattr_regression_test_exempt_mask;
		}
	}
	if (!(a->mask & cattr_test_exempt_mask))
	{
		a->test_exempt = ps->default_test_exemption;
		a->mask |= cattr_test_exempt_mask;
	}
	if (!(a->mask & cattr_test_baseline_exempt_mask))
	{
		a->test_baseline_exempt = ps->default_test_exemption;
		a->mask |= cattr_test_baseline_exempt_mask;
	}
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
	int		edit;

	trace(("new_change_main()\n{\n"/*}*/));
	cattr_data = 0;
	project_name = 0;
	edit = 0;
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
			cattr_verify(arglex_value.alv_string, cattr_data);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				new_change_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_edit:
			if (edit)
				fatal("duplicate \"%s\" option", arglex_value.alv_string);
			edit++;
			break;
		}
		arglex();
	}
	if (!edit && !cattr_data)
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
	 * see if must invoke editor
	 */
	if (edit)
	{
		/*
		 * build template cattr
		 */
		if (!cattr_data)
		{
			string_ty *none = str_from_c("none");
			cattr_data = (cattr)cattr_type.alloc();
			cattr_data->brief_description = str_copy(none);
			cattr_data->description = str_copy(none);
			cattr_data->cause = change_cause_internal_bug;
			str_free(none);
		}

		/*
		 * default a few things
		 */
		pstate_data = project_pstate_get(pp);
		cattr_defaults(cattr_data, pstate_data);

		/*
		 * edit the attributes
		 */
		cattr_edit(&cattr_data);
	}

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
		/*
		 * If they are asking for default behaviour, don't complain.
		 * (e.g. admin may have given general testing exemption)
		 */
		cattr dflt = cattr_type.alloc();
		dflt->cause = cattr_data->cause;
		cattr_defaults(dflt, pstate_data);

		if
		(
			(
				(cattr_data->mask & cattr_test_exempt_mask)
			&&
				cattr_data->test_exempt
			&&
				(cattr_data->test_exempt != dflt->test_exempt)
			)
		||
			(
				(cattr_data->mask & cattr_test_baseline_exempt_mask)
			&&
				cattr_data->test_baseline_exempt
			&&
				(cattr_data->test_baseline_exempt != dflt->test_baseline_exempt)
			)
		||
			(
				(cattr_data->mask & cattr_regression_test_exempt_mask)
			&&
				cattr_data->regression_test_exempt
			&&
				(cattr_data->regression_test_exempt != dflt->regression_test_exempt)
			)
		)
			fatal("developers may not grant testing exemptions");
		cattr_type.free(dflt);
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
	assert(cattr_data->mask & cattr_cause_mask);
	if (change_number == 1)
	{
		cattr_data->cause = change_cause_internal_enhancement;
		cattr_data->test_baseline_exempt = 1;
		cattr_data->mask |= cattr_test_baseline_exempt_mask;
		cattr_data->regression_test_exempt = 1;
		cattr_data->mask |= cattr_regression_test_exempt_mask;
	}
	cattr_defaults(cattr_data, pstate_data);
	cstate_data->cause = cattr_data->cause;
	assert(cattr_data->mask & cattr_test_exempt_mask);
	cstate_data->test_exempt = cattr_data->test_exempt;
	assert(cattr_data->mask & cattr_test_baseline_exempt_mask);
	cstate_data->test_baseline_exempt =
		cattr_data->test_baseline_exempt;
	assert(cattr_data->mask & cattr_regression_test_exempt_mask);
	cstate_data->regression_test_exempt =
		cattr_data->regression_test_exempt;
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
