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
 * MANIFEST: functions to implement review pass
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ael.h>
#include <aerp.h>
#include <arglex2.h>
#include <change.h>
#include <commit.h>
#include <dir.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void review_pass_usage _((void));

static void
review_pass_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -Review_PASS <change_number> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Review_PASS -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Review_PASS -Help\n", progname);
	quit(1);
}


static void review_pass_help _((void));

static void
review_pass_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Review_PASS - pass a change review",
"",
"SYNOPSIS",
"	%s -Review_PASS [ <option>... ]",
"	%s -Review_PASS -List [ <option>... ]",
"	%s -Review_PASS -Help",
"",
"DESCRIPTION",
"	The %s -Review_PASS command is used to notify",
"	%s that a change has passed review.",
"",
"	The change will be advenced from the 'being_reviewed'",
"	state to the 'awaitiong_integration' state.",
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
"	csh%%	alias aerp '%s -rp \\!* -v'",
"	sh$	aerp(){%s -rp $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in the",
"	'being_reviewed' state.",
"	It is an error if the current user is not a reviewer of",
"	the project.",
"	Its is an error if the current user developed the change",
"	and the project is configured to disallow developers to",
"	review their own changes (default).",
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

	help(text, SIZEOF(text), review_pass_usage);
}


static void review_pass_list _((void (*usage)(void)));

static void
review_pass_list(usage)
	void		(*usage)_((void));
{
	string_ty	*project_name;

	trace(("review_pass_list()\n{\n"/*}*/));
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
		1 << cstate_state_being_reviewed
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void review_pass_main _((void));

static void
review_pass_main()
{
	cstate		cstate_data;
	pstate		pstate_data;
	cstate_history	history_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;

	trace(("review_pass_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(review_pass_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				review_pass_usage();
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
				review_pass_usage();
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
	change_cstate_lock_prepare(cp);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * it is an error if the change is not in the 'being_reviewed' state.
	 */
	if (cstate_data->state != cstate_state_being_reviewed)
		change_fatal(cp, "not in 'being_reviewed' state");
	if (!project_reviewer_query(pp, user_name(up)))
	{
		project_fatal
		(
			pp,
			"user \"%S\" is not a reviewer",
			user_name(up)
		);
	}
	if
	(
		!pstate_data->developer_may_review
	&&
		str_equal(change_developer_name(cp), user_name(up))
	)
		change_fatal(cp, "developer may not review");

	/*
	 * change the state
	 * remember who reviewed it
	 * add to the change's history
	 */
	cstate_data->state = cstate_state_awaiting_integration;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_review_pass;

	/*
	 * write out the data and release the locks
	 */
	change_cstate_write(cp);
	commit();
	lock_release();

	/*
	 * run the notify command
	 */
	change_run_review_pass_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, "passed review");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
review_pass()
{
	trace(("review_pass()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		review_pass_main();
		break;

	case arglex_token_help:
		review_pass_help();
		break;

	case arglex_token_list:
		review_pass_list(review_pass_usage);
		break;
	}
	trace((/*{*/"}\n"));
}
