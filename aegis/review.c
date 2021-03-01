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
 * MANIFEST: review a change, pass or fail or undo
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <arglex2.h>
#include <change.h>
#include <commit.h>
#include <dir.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <list.h>
#include <lock.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <review.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void review_pass_usage _((void));

static void
review_pass_usage()
{
	char		*progname;

	progname = option_get_progname();
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


static void review_list _((void (*usage)(void)));

static void
review_list(usage)
	void		(*usage)_((void));
{
	string_ty	*project_name;

	trace(("review_list()\n{\n"/*}*/));
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
		review_list(review_pass_usage);
		break;
	}
	trace((/*{*/"}\n"));
}


static void review_pass_undo_usage _((void));

static void
review_pass_undo_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -Review_Pass_Undo <change_number> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Review_Pass_Undo -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Review_Pass_Undo -Help\n", progname);
	quit(1);
}


static void review_pass_undo_help _((void));

static void
review_pass_undo_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Review_Pass_Undo - rescind a change review pass",
"",
"SYNOPSIS",
"	%s -Review_Pass_Undo [ <option>... ]",
"	%s -Review_Pass_Undo -List [ <option>... ]",
"	%s -Review_Pass_Undo -Help",
"",
"DESCRIPTION",
"	The %s -Review_Pass_Undo command is used to notify %s",
"	that a change review pass has been rescinded.",
"",
"	The change will be moved from the 'awaiting_integration'",
"	state to the 'being_reviewed' state.",
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
"		 This option may be used to obtain more information",
"		 about how to use the %s program.",
"",
"	-List",
"		 This option may be used to obtain a list of suitable",
"		 subjects for this command.  The list may be more",
"		 general than expected.",
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
"		 This option may be used to cause listings to produce",
"		 the bare minimum of information.  It is usually",
"		 useful for shell scripts.",
"",
"	-Verbose",
"		 This option may be used to cause %s to produce",
"		 more output.  By default %s only produces output",
"		 on errors.  When used with the -List option this",
"		 option causes column headings to be added.",
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
"	csh%%	alias aerpu '%s -rp \\!* -v'",
"	sh$	aerpu(){%s -rp $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in the",
"	'awaiting_integration' state.",
"	It is an error if the current user is not the reviewer of",
"	the change.",
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
"NAME",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
	};

	help(text, SIZEOF(text), review_pass_undo_usage);
}


static void review_pass_undo_list _((void (*usage)(void)));

static void
review_pass_undo_list(usage)
	void		(*usage)_((void));
{
	string_ty	*project_name;

	trace(("review_list()\n{\n"/*}*/));
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
		1 << cstate_state_awaiting_integration
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void review_pass_undo_main _((void));

static void
review_pass_undo_main()
{
	cstate		cstate_data;
	pstate		pstate_data;
	cstate_history	history_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;

	trace(("review_pass_undo_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(review_pass_undo_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				review_pass_undo_usage();
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
				review_pass_undo_usage();
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
	 * it is an error if the current user is not the original reviewer
	 */
	if (cstate_data->state != cstate_state_awaiting_integration)
		change_fatal(cp, "not in 'awaiting_integration' state");
	if (!str_equal(change_reviewer_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
			"user \"%S\" is not the reviewer",
			user_name(up)
		);
	}

	/*
	 * change the state
	 * add to the change's history
	 */
	cstate_data->state = cstate_state_being_reviewed;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_review_pass_undo;

	/*
	 * write out the data and release the locks
	 */
	change_cstate_write(cp);
	commit();
	lock_release();

	/*
	 * run the notify command
	 */
	change_run_review_pass_undo_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, "review pass rescinded");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
review_pass_undo()
{
	trace(("review_pass_undo()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		review_pass_undo_main();
		break;

	case arglex_token_help:
		review_pass_undo_help();
		break;

	case arglex_token_list:
		review_pass_undo_list(review_pass_undo_usage);
		break;
	}
	trace((/*{*/"}\n"));
}


static void review_fail_usage _((void));

static void
review_fail_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -Review_FAIL <commentfile> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Review_FAIL -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Review_FAIL -Help\n", progname);
	quit(1);
}


static void review_fail_help _((void));

static void
review_fail_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Review_FAIL - fail a change review",
"",
"SYNOPSIS",
"	%s -Review_FAIL <reason-file> [ <option>... ]",
"	%s -Review_FAIL -List [ <option>... ]",
"	%s -Review_FAIL -Help",
"",
"DESCRIPTION",
"	The %s -Review_FAIL command is used to inform",
"	%s that a change has failed review.",
"",
"	The change will be returned from the 'being_reviewed'",
"	state to the 'being_developed' state.  The change will",
"	cease to be assigned to the current user, and will be",
"	reassigned to the originating developer.",
"",
"	The developer will be notified by mail.	 See the",
"	integrate_fail_notify_command in aepconf(5) for more",
"	information.",
"",
"	The reason-file will contain a description of why the",
"	change was failed.  The file is in plain text.	It is",
"	recommened that you only use newline to terminate",
"	paragraphs, as with will result in better formatting in",
"	the various listings.",
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
"	csh%%	alias aerf '%s -rf \\!* -v'",
"	sh$	aerf(){%s -rf $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in the",
"	'being_reviewed' state.",
"	It is an error if the current user is not a reviewer for",
"	the project.",
"	It is an error if the current user developed the change",
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

	help(text, SIZEOF(text), review_fail_usage);
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


static void rf_func _((void *, dir_walk_message_ty, string_ty *, struct stat *));

static void
rf_func(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	change_ty	*cp;
	int		uid;

	trace(("rf_func(message = %d, path = %08lX, st = %08lX)\n{\n"/*}*/,
		message, path, st));
	cp = (change_ty *)arg;
	trace_string(path->str_text);
	switch (message)
	{
	case dir_walk_file:
	case dir_walk_dir_before:
		os_become_query(&uid, (int *)0, (int *)0);
		if (st->st_uid == uid)
		{
			os_chmod(path, st->st_mode | 0200);
			undo_chmod(path, st->st_mode);
		}
		repair_diff_time(cp, path);
		break;

	case dir_walk_dir_after:
	case dir_walk_special:
	case dir_walk_symlink:
		break;
	}
	trace((/*{*/"}\n"));
}


static void review_fail_main _((void));

static void
review_fail_main()
{
	string_ty	*dd;
	cstate		cstate_data;
	pstate		pstate_data;
	cstate_history	history_data;
	string_ty	*comment = 0;
	int		j;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	user_ty		*devup;

	trace(("review_fail_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(review_fail_usage);
			continue;

		case arglex_token_string:
			if (comment)
				fatal("too many files named");
			os_become_orig();
			comment = read_whole_file(arglex_value.alv_string);
			os_become_undo();
			break;

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
				review_fail_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (!comment)
		fatal("no comment file supplied");

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
	lock_prepare_ustate_all(); /* we don't know which user until later */
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
	cstate_data->state = cstate_state_being_developed;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_review_fail;
	history_data->why = comment;
	cstate_data->build_time = 0;
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;

	/*
	 * add it back into the user's change list
	 */
	devup = user_symbolic(pp, change_developer_name(cp));
	user_own_add(devup, project_name_get(pp), change_number);

	/*
	 * Restore write permission to the change files
	 * and repair the diff time fields.
	 */
	dd = change_development_directory_get(cp, 1);
	user_become(devup);
	dir_walk(dd, rf_func, cp);
	os_become_undo();

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
	 * write out the data and release the locks
	 */
	project_pstate_write(pp);
	change_cstate_write(cp);
	user_ustate_write(devup);
	commit();
	lock_release();

	/*
	 * run the notify command
	 */
	change_run_review_fail_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, "failed review, returned to developer");
	change_free(cp);
	project_free(pp);
	user_free(up);
	user_free(devup);
	trace((/*{*/"}\n"));
}


void
review_fail()
{
	trace(("review_fail()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		review_fail_main();
		break;

	case arglex_token_help:
		review_fail_help();
		break;

	case arglex_token_list:
		review_list(review_fail_usage);
		break;
	}
	trace((/*{*/"}\n"));
}
