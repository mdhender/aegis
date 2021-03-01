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
 * MANIFEST: functions to add and remove reviewers
 */

#include <stdio.h>
#include <stdlib.h>

#include <arglex2.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <list.h>
#include <lock.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <reviewer.h>
#include <trace.h>
#include <user.h>
#include <word.h>


static void new_reviewer_usage _((void));

static void
new_reviewer_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -New_Reviewer <username>... [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Reviewer -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Reviewer -Help\n", progname);
	quit(1);
}


static void new_reviewer_help _((void));

static void
new_reviewer_help()
{
	static char *text[] =
	{
"NAME",
"	%s -New_ReViewer - add new reviewers to a project",
"",
"SYNOPSIS",
"	%s -New_ReViewer <user-name>... [ <option>... ]",
"	%s -New_ReViewer -List [ <option>... ]",
"	%s -New_ReViewer -Help",
"",
"DESCRIPTION",
"	The %s -New_ReViewer command is used to add new reviewers",
"	to a project.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Help",
"		This option may be used to obtain more information",
"		about how to use the %s program.",
"",
"	-List",
"		This option may be used to obtain a list of suitable",
"		subjects for this command.  The list may be more",
"		general than expected.",
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
"	csh%%	alias aenrv '%s -nrv \\!* -v'",
"	sh$	aenrv(){%s -nrv $* -v}",
"",
"ERRORS",
"	It is an error if the current user is not an administrator",
"	of the project.",
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

	help(text, SIZEOF(text), new_reviewer_usage);
}


static void reviewer_list _((void (*)(void)));

static void
reviewer_list(usage)
	void	(*usage)_((void));
{
	string_ty	*project_name;

	trace(("reviewer_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
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
	list_reviewers(project_name, 0);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void new_reviewer_main _((void));

static void
new_reviewer_main()
{
	wlist		wl;
	string_ty	*s1;
	pstate		pstate_data;
	int		j;
	string_ty	*project_name;
	project_ty	*pp;
	user_ty		*up;

	trace(("new_reviewer_main()\n{\n"/*}*/));
	project_name = 0;
	wl_zero(&wl);
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_reviewer_usage);
			continue;

		case arglex_token_string:
			s1 = str_from_c(arglex_value.alv_string);
			if (wl_member(&wl, s1))
				fatal("user \"%s\" named more than once", s1->str_text);
			wl_append(&wl, s1);
			str_free(s1);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				new_reviewer_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (!wl.wl_nwords)
		fatal("no users named");

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
	 * lock the project for change
	 */
	project_pstate_lock_prepare(pp);
	lock_take();
	pstate_data = project_pstate_get(pp);

	/*
	 * check they are allowed to do this
	 */
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
	 * check they they are OK users
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		user_ty		*candidate;

		/*
		 * make sure the user isn't already there
		 */
		candidate = user_symbolic(pp, wl.wl_word[j]);
		if (project_reviewer_query(pp, user_name(candidate)))
		{
			project_fatal
			(
				pp,
				"user \"%S\" is already a reviewer",
				user_name(candidate)
			);
		}

		/*
		 * make sure the user exists
		 *	(should we check s/he is in the project's group?)
		 * this is to avoid security holes
		 */
		if (!user_uid_check(user_name(candidate)))
		{
			fatal
			(
				"user \"%s\" is too privileged",
				user_name(candidate)->str_text
			);
		}

		/*
		 * add them to the list
		 */
		project_reviewer_add(pp, user_name(candidate));
		user_free(candidate);
	}

	/*
	 * write out and release lock
	 */
	project_pstate_write(pp);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		project_verbose
		(
			pp,
			"user \"%S\" is now a reviewer",
			wl.wl_word[j]
		);
	}
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
new_reviewer()
{
	trace(("new_reviewer()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		new_reviewer_main();
		break;

	case arglex_token_help:
		new_reviewer_help();
		break;

	case arglex_token_list:
		reviewer_list(new_reviewer_usage);
		break;
	}
	trace((/*{*/"}\n"));
}


static void remove_reviewer_usage _((void));

static void
remove_reviewer_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -Remove_Reviewer [ <option>... ] <username>...\n", progname);
	fprintf(stderr, "       %s -Remove_Reviewer -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Remove_Reviewer -Help\n", progname);
	quit(1);
}


static void remove_reviewer_help _((void));

static void
remove_reviewer_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Remove_ReViewer - remove reviewers from a project",
"",
"SYNOPSIS",
"	%s -Remove_ReViewer <user-name>... [ <option>... ]",
"	%s -Remove_ReViewer -List [ <option>... ]",
"	%s -Remove_ReViewer -Help",
"",
"DESCRIPTION",
"	The %s -Remove_ReViewer command is used to remove",
"	reviewers from a project.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Help",
"		This option may be used to obtain more information",
"		about how to use the %s program.",
"",
"	-List",
"		This option may be used to obtain a list of suitable",
"		subjects for this command.  The list may be more",
"		general than expected.",
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
"	csh%%	alias aerrv '%s -rrv \\!* -v'",
"	sh$	aerrv(){%s -rrv $* -v}",
"",
"ERRORS",
"	It is an error if the current user is not an administrator",
"	of the project.",
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

	help(text, SIZEOF(text), remove_reviewer_usage);
}


static void remove_reviewer_main _((void));

static void
remove_reviewer_main()
{
	wlist		wl;
	string_ty	*s1;
	pstate		pstate_data;
	int		j;
	string_ty	*project_name;
	project_ty	*pp;
	user_ty		*up;

	trace(("remove_reviewer_main()\n{\n"/*}*/));
	project_name = 0;
	wl_zero(&wl);
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(remove_reviewer_usage);
			continue;

		case arglex_token_string:
			s1 = str_from_c(arglex_value.alv_string);
			if (wl_member(&wl, s1))
				fatal("user \"%s\" named more than once", s1->str_text);
			wl_append(&wl, s1);
			str_free(s1);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				remove_reviewer_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (!wl.wl_nwords)
		fatal("no users named");

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
	 * lock the project for change
	 */
	project_pstate_lock_prepare(pp);
	lock_take();
	pstate_data = project_pstate_get(pp);

	/*
	 * check they are allowed to do this
	 */
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
	 * check they they are OK users
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		user_ty	*candidate;

		candidate = user_symbolic(pp, wl.wl_word[j]);
		if (!project_reviewer_query(pp, user_name(candidate)))
		{
			project_fatal
			(
				pp,
				"user \"%S\" is not a reviewer",
				user_name(candidate)
			);
		}
		project_reviewer_delete(pp, user_name(candidate));
		user_free(candidate);
	}

	/*
	 * write out and release lock
	 */
	project_pstate_write(pp);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		project_verbose
		(
			pp,
			"user \"%S\" is no longer a reviewer",
			wl.wl_word[j]
		);
	}
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
remove_reviewer()
{
	trace(("remove_reviewer()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		remove_reviewer_main();
		break;

	case arglex_token_help:
		remove_reviewer_help();
		break;

	case arglex_token_list:
		reviewer_list(remove_reviewer_usage);
		break;
	}
	trace((/*{*/"}\n"));
}
