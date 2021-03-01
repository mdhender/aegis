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
 * MANIFEST: functions to implement remove file
 */

#include <stdio.h>
#include <stdlib.h>

#include <ael.h>
#include <aerm.h>
#include <arglex2.h>
#include <change.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <user.h>
#include <word.h>


/*
 * NAME
 *	remove_file_usage
 *
 * SYNOPSIS
 *	void remove_file_usage(void);
 *
 * DESCRIPTION
 *	The remove_file_usage function is used to
 *	tell the user how to use the 'aegis -ReMove_file' command.
 */

static void remove_file_usage _((void));

static void
remove_file_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -ReMove_file <filename>... [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -ReMove_file -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -ReMove_file -Help\n", progname);
	quit(1);
}


/*
 * NAME
 *	remove_file_help
 *
 * SYNOPSIS
 *	void remove_file_help(void);
 *
 * DESCRIPTION
 *	The remove_file_help function is used to
 *	describe the 'aegis -ReMove_file' command to the user.
 */

static void remove_file_help _((void));

static void
remove_file_help()
{
	static char *text[] =
	{
"NAME",
"	%s -ReMove_file - add files to be deleted to a chasnge",
"",
"SYNOPSIS",
"	%s -ReMove_file <filename>... [ <option>... ]",
"	%s -ReMove_file -List [ <option>... ]",
"	%s -ReMove_file -Help",
"",
"DESCRIPTION",
"	The %s -ReMove_file command is used to add files to be",
"	deleted to a change.",
"",
"	The %s program will attempt to intuit the file names",
"	intended.  All file names are stored within %s as",
"	relative to the root of the baseline directory tree.  The",
"	development directory and the integration directory are",
"	shadows of the baseline dirdctory, and so these relative",
"	names aply there, too.  Files named on the command line",
"	are first converted to absolute paths if necessary.  They",
"	are then compared with the baseline path, and the",
"	development directory path, and the integration directory",
"	path, to determine a root-relative name.  It is an error",
"	if the file named is outside one of these directory",
"	trees.",
"",
"	The file will be added to the list of files in the",
"	change, and will be remoed from the baseline at",
"	integration time.",
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
"	csh%%	alias aerm '%s -rm \\!* -v'",
"	sh$	aerm(){%s -rm $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in the",
"	'being_developed' state.",
"	It is an error if the change is not assigned to the",
"	current user.",
"	It is an error if the filesa does not exist in the",
"	baseline.",
"	It is an error if the file is already part of the change.",
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

	help(text, SIZEOF(text), remove_file_usage);
}


/*
 * NAME
 *	remove_file_list
 *
 * SYNOPSIS
 *	void remove_file_list(void);
 *
 * DESCRIPTION
 *	The remove_file_list function is used to
 *	list the file the user may wish to add to the change
 *	as a deletion.  All project files are listed.
 */

static void remove_file_list _((void));

static void
remove_file_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("remove_file_list()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(remove_file_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				remove_file_usage();
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
				remove_file_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_project_files(project_name, change_number);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	remove_file_main
 *
 * SYNOPSIS
 *	void remove_file_main(void);
 *
 * DESCRIPTION
 *	The remove_file_main function is used to
 *	add a file to a change as a deletion.
 *	The file will be deleted from the baseline on successful
 *	integration of the change.
 *
 *	The names of the relevant files are gleaned from the command line.
 */

static void remove_file_main _((void));

static void
remove_file_main()
{
	wlist		wl;
	string_ty	*s1;
	string_ty	*s2;
	cstate		cstate_data;
	cstate_src	c_src_data;
	pstate_src	p_src_data;
	int		j;
	pconf		pconf_data;
	pstate		pstate_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;
	string_ty	*dd;
	string_ty	*bl;

	trace(("remove_file_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	wl_zero(&wl);
	nolog = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(remove_file_usage);
			continue;

		case arglex_token_string:
			s1 = str_from_c(arglex_value.alv_string);
			os_become_orig();
			s2 = os_pathname(s1, 1);
			os_become_undo();
			if (wl_member(&wl, s2))
				fatal("file \"%s\" named more than once", arglex_value.alv_string);
			wl_append(&wl, s2);
			str_free(s2);
			str_free(s1);
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				remove_file_usage();
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
				remove_file_usage();
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
	if (!wl.wl_nwords)
		fatal("no files specified");

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
	 * take the locks and read the change state
	 */
	change_cstate_lock_prepare(cp);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pconf_data = change_pconf_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * It is an error if the change is not in the in_development state.
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
	 * resolve the path of each file
	 * 1.	the absolute path of the file name is obtained
	 * 2.	if the file is inside the development directory, ok
	 * 3.	if the file is inside the baseline, ok
	 * 4.	if neither, error
	 */
	dd = change_development_directory_get(cp, 1);
	bl = project_baseline_path_get(pp, 1);
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		s1 = wl.wl_word[j];
		assert(s1->str_text[0] == '/');
		s2 = os_below_dir(dd, s1);
		if (!s2)
			s2 = os_below_dir(bl, s1);
		if (!s2)
			change_fatal(cp, "path \"%S\" unrelated", s1);
		str_free(s1);
		wl.wl_word[j] = s2;
	}

	/*
	 * ensure that each file
	 * 1. is not already part of the change
	 * 2. is in the baseline
	 * add it to the change
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		s1 = wl.wl_word[j];
		if (change_src_find(cp, s1))
			change_fatal(cp, "file \"%S\" already in change", s1);
		p_src_data = project_src_find(pp, s1);
		if (!p_src_data || p_src_data->deleted_by)
			project_fatal(pp, "file \"%S\" does not exist", s1);
		c_src_data = change_src_new(cp);
		c_src_data->file_name = str_copy(s1);
		c_src_data->action = file_action_remove;
		c_src_data->usage = p_src_data->usage;
		c_src_data->edit_number = str_copy(p_src_data->edit_number);
	}

	/*
	 * the number of files changed, or the version did,
	 * so stomp on the validation fields.
	 */
	cstate_data->build_time = 0;
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;
	cstate_data->regression_test_time = 0;

	/*
	 * write the data and release the lock
	 */
	change_cstate_write(cp);
	commit();
	lock_release();

	/*
	 * run the change file command
	 */
	if (!nolog)
		log_open(change_logfile_get(cp), up);
	change_run_change_file_command(cp, &wl, up);

	/*
	 * verbose success message
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
		change_verbose(cp, "file \"%S\" remove file", wl.wl_word[j]);
	wl_free(&wl);
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	remove_file
 *
 * SYNOPSIS
 *	void remove_file(void);
 *
 * DESCRIPTION
 *	The remove_file function is used to
 *	dispatch the 'aegis -ReMove_file' command to the relevant
 *	function to do it's work.
 */

void
remove_file()
{
	trace(("remove_file()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		remove_file_main();
		break;

	case arglex_token_help:
		remove_file_help();
		break;

	case arglex_token_list:
		remove_file_list();
		break;
	}
	trace((/*{*/"}\n"));
}
