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
 * MANIFEST: copy files into a change, and undo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <aecpu.h>
#include <ael.h>
#include <arglex2.h>
#include <commit.h>
#include <change.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <word.h>


static void copy_file_undo_usage _((void));

static void
copy_file_undo_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -CoPy_file_Undo [ <option>... ] <filename>...\n", progname);
	fprintf(stderr, "       %s -CoPy_file_Undo -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -CoPy_file_Undo -Help\n", progname);
	quit(1);
}


static void copy_file_undo_help _((void));

static void
copy_file_undo_help()
{
	static char *text[] =
	{
"NAME",
"	%s -CoPy_file_Undo - reverse action of aecp",
"",
"SYNOPSIS",
"	%s -CoPy_file_Undo [ <option>... ] <filename>...",
"	%s -CoPy_file_Undo -List [ <option>... ]",
"	%s -CoPy_file_Undo -Help",
"",
"DESCRIPTION",
"	The %s -CoPy_file_Undo command is used to remove a",
"	file previously copied into a change.",
"",
"	The %s program will attempt to intuit the file names",
"	intended.  All file names are stored within %s as",
"	relative to the root of the baseline directory tree.  The",
"	development directory and the integration directory are",
"	shadows of the baseline directory, and so these relative",
"	names aply there, too.	Files named on the command line",
"	are first converted to absolute paths if necessary.  They",
"	are then compared with the baseline path, and the",
"	development directory path, and the integration directory",
"	path, to determine a root-relative name.  It is an error",
"	if the file named is outside one of these directory",
"	trees.",
"",
"	The named files will be removed from the list of files in",
"	the change.  The file is deleted from the development",
"	directory unless the -Keep option is specified.  The -",
"	Keep option should be used with great care, as you can",
"	confuse tools such as make(1) by leaving these files in",
"	place.",
"",
"	This command will cancel any build or test registrations,",
"	because deleting a file logically invalidates them.  If the",
"	config file was deleted, any diff registration will also be",
"	cancelled.",
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
"	csh%%	alias aecpu '%s -cpu \\!* -v'",
"	sh$	aecpu(){%s -cpu $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in the",
"	'being_developed' state.",
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

	help(text, SIZEOF(text), copy_file_undo_usage);
}


static void copy_file_undo_list _((void));

static void
copy_file_undo_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("copy_file_undo_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(copy_file_undo_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				copy_file_undo_usage();
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
				copy_file_undo_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_change_files(project_name, change_number);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void copy_file_undo_main _((void));

static void
copy_file_undo_main()
{
	wlist		wl;
	cstate		cstate_data;
	pstate		pstate_data;
	int		j;
	string_ty	*s1;
	string_ty	*s2;
	int		keep;
	pconf		pconf_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;
	int		config_seen;
	string_ty	*config_name;

	trace(("copy_file_undo_main()\n{\n"/*}*/));
	wl_zero(&wl);
	project_name = 0;
	change_number = 0;
	keep = 0;
	nolog = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(copy_file_undo_usage);
			continue;

		case arglex_token_string:
			s1 = str_from_c(arglex_value.alv_string);
			os_become_orig();
			s2 = os_pathname(s1, 1);
			os_become_undo();
			str_free(s1);
			if (wl_member(&wl, s2))
				fatal("file \"%s\" named more than once", arglex_value.alv_string);
			wl_append(&wl, s2);
			str_free(s2);
			break;

		case arglex_token_keep:
			if (keep)
			{
				duplicate:
				fatal("duplicate %s option", arglex_value.alv_string);
			}
			keep = 1;
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				copy_file_undo_usage();
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
				copy_file_undo_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (nolog)
				goto duplicate;
			nolog = 1;
			break;
		}
		arglex();
	}
	if (!wl.wl_nwords)
		fatal("no files named");

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
	 * lock the change file
	 */
	change_cstate_lock_prepare(cp);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);
	pconf_data = change_pconf_get(cp);

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
	config_seen = 0;
	config_name = str_from_c(THE_CONFIG_FILE);
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		s1 = wl.wl_word[j];
		assert(s1->str_text[0] == '/');
		s2 = os_below_dir(change_development_directory_get(cp, 1), s1);
		if (!s2)
			s2 = os_below_dir(project_baseline_path_get(pp, 1), s1);
		if (!s2)
			change_fatal(cp, "path \"%S\" unrelated", s1);
		str_free(s1);
		wl.wl_word[j] = s2;
		if (str_equal(s2, config_name))
			config_seen++;
	}
	str_free(config_name);

	/*
	 * ensure that each file
	 * 1. is already part of the change
	 * 2. is being modified by this change
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		cstate_src	src_data;

		s1 = wl.wl_word[j];
		src_data = change_src_find(cp, s1);
		if (!src_data)
			change_fatal(cp, "file \"%S\" not in change", s1);
		if (src_data->action != file_action_modify)
			change_fatal(cp, "file \"%S\" is not -CoPy_file", s1);
		if (config_seen)
			src_data->diff_time = 0;
	}

	/*
	 * Remove each file from the development directory,
	 * if it still exists.
	 * Remove the difference file, too.
	 */
	if (!keep)
	{
		string_ty	*dd;

		dd = change_development_directory_get(cp, 1);
		user_become(up);
		for (j = 0; j < wl.wl_nwords; ++j)
		{
			s1 = wl.wl_word[j];
			s2 = str_format("%S/%S", dd, s1);
			if (os_exists(s2))
				commit_unlink_errok(s2);
			str_free(s2);

			s2 = str_format("%S/%S,D", dd, s1);
			if (os_exists(s2))
				commit_unlink_errok(s2);
			str_free(s2);
		}
		user_become_undo();
	}

	/*
	 * Remove each file to the change file,
	 * and write it back out.
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
		change_src_remove(cp, wl.wl_word[j]);

	/*
	 * the number of files changed, or the version did,
	 * so stomp on the validation fields.
	 */
	cstate_data->build_time = 0;
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;
	cstate_data->regression_test_time = 0;

	/*
	 * release the locks
	 */
	change_cstate_write(cp);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
		change_verbose(cp, "file \"%S\" copy file undo", wl.wl_word[j]);

	/*
	 * run the change file command
	 */
	if (!nolog)
		log_open(change_logfile_get(cp), up);
	change_run_change_file_command(cp, &wl, up);
	wl_free(&wl);
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
copy_file_undo()
{
	trace(("copy_file_undo()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		copy_file_undo_main();
		break;

	case arglex_token_help:
		copy_file_undo_help();
		break;

	case arglex_token_list:
		copy_file_undo_list();
		break;
	}
	trace((/*{*/"}\n"));
}
