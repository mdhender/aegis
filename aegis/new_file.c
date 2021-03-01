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
 * MANIFEST: functions to add or remove a new file to a change
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <arglex2.h>
#include <change.h>
#include <col.h>
#include <commit.h>
#include <error.h>
#include <glue.h>
#include <help.h>
#include <list.h>
#include <lock.h>
#include <log.h>
#include <new_file.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <user.h>
#include <word.h>


static void new_file_usage _((void));

static void
new_file_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -New_File <filename>... [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_File -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_File -Help\n", progname);
	quit(1);
}


static void new_file_help _((void));

static void
new_file_help()
{
	static char *text[] =
	{
"NAME",
"	%s -New_File - add new files to a change",
"",
"SYNOPSIS",
"	%s -New_File <file-name>... [ <option>... ]",
"	%s -New_File -List [ <option>... ]",
"	%s -New_File -Help",
"",
"DESCRIPTION",
"	The %s -New_File command is used to add new files to a",
"	change.",
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
"	The named files will be added to the list of files in the",
"	change.  For each file named, a new file is created in the",
"	development directory, if it does not exist already.  The",
"	config file will be searched for a template for the new",
"	file.  If a template is found, the new file will be",
"	initialized to the template, otherwise it will be created",
"	empty.  If the file already exists, it will not be altered.",
"	See aepconf(5) for more information.",
"",
"OPTIONS",
"	The following options are understood",
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
"	csh%%	alias aenf '%s -nf \\!* -v'",
"	sh$	aenf(){%s -nf $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in the",
"	'being_developed' state.",
"	It is an error if the change is not assigned to the",
"	current user.",
"	It is an error if the file is already part of the change.",
"	It is an error if the file is already part of the",
"	baseline.",
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

	help(text, SIZEOF(text), new_file_usage);
}


static void new_file_list _((void));

static void
new_file_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("new_file_list()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_file_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				new_file_usage();
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
				new_file_usage();
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


static void new_file_main _((void));

static void
new_file_main()
{
	string_ty	*bl;
	string_ty	*dd;
	wlist		wl;
	cstate		cstate_data;
	pstate		pstate_data;
	int		j;
	string_ty	*s1;
	string_ty	*s2;
	pconf		pconf_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;

	trace(("new_file_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	wl_zero(&wl);
	nolog = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_file_usage);
			continue;

		case arglex_token_string:
			s1 = str_from_c(arglex_value.alv_string);
			os_become_orig();
			s2 = os_pathname(s1, 1);
			str_free(s1);
			if (wl_member(&wl, s2))
				fatal("file \"%s\" named more than once", arglex_value.alv_string);
			s1 = os_entryname(s2);
			os_become_undo();
			if (s1->str_length > PATH_ELEMENT_MAX - 2)
			{
				fatal
				(
					"file \"%s\" basename too long (by %ld)",
					arglex_value.alv_string,
					s1->str_length - (PATH_ELEMENT_MAX - 2)
				);
			}
			str_free(s1);
			wl_append(&wl, s2);
			str_free(s2);
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				new_file_usage();
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
				new_file_usage();
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
	 * 2. is not already part of the baseline
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		pstate_src	src_data;

		s1 = wl.wl_word[j];
		if (change_src_find(cp, s1))
			change_fatal(cp, "file \"%S\" already exists", s1);
		src_data = project_src_find(pp, s1);
		if
		(
			src_data
		&&
			!src_data->about_to_be_created_by
		&&
			!src_data->deleted_by
		)
			project_fatal(pp, "file \"%S\" already exists", s1);
	}

	/*
	 * Create each file in the development directory,
	 * if it does not already exist.
	 * Create any necessary directories along the way.
	 */
	user_become(up);
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		s1 = wl.wl_word[j];
		os_mkdir_between(dd, s1, 02755);
		s2 = str_format("%S/%S", dd, s1);
		if (!os_exists(s2))
		{
			int		fd;
			string_ty	*template;

			user_become_undo();
			template = change_file_template(cp, s1);
			user_become(up);
			fd = glue_creat(s2->str_text, 0666);
			if (fd < 0)
				nfatal("create(\"%s\")", s2->str_text);
			if (template)
			{
				glue_write
				(
					fd,
					template->str_text,
					template->str_length
				);
				str_free(template);
			}
			glue_close(fd);
			os_chmod(s2, 0644 & ~change_umask(cp));
		}
		str_free(s2);
	}
	user_become_undo();

	/*
	 * Add each file to the change file,
	 * and write it back out.
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		cstate_src	src_data;

		src_data = change_src_new(cp);
		src_data->file_name = str_copy(wl.wl_word[j]);
		src_data->action = file_action_create;
		src_data->usage = file_usage_source;
	}

	/*
	 * the number of files changed, or the version did,
	 * so stomp on the validation fields.
	 */
	cstate_data->build_time = 0;
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;

	/*
	 * release the locks
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
		change_verbose(cp, "file \"%S\" added", wl.wl_word[j]);
	wl_free(&wl);
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
new_file()
{
	trace(("new_file()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		new_file_main();
		break;

	case arglex_token_help:
		new_file_help();
		break;

	case arglex_token_list:
		new_file_list();
		break;
	}
	trace((/*{*/"}\n"));
}


static void new_file_undo_usage _((void));

static void
new_file_undo_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -New_File_Undo <filename>... [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_File_Undo -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_File_Undo -Help\n", progname);
	quit(1);
}


static void new_file_undo_help _((void));

static void
new_file_undo_help()
{
	static char *text[] =
	{
"NAME",
"	%s -New_File_Undo - remove new files form a change",
"",
"SYNOPSIS",
"	%s -New_File_Undo <file-name>... [ <option>... ]",
"	%s -New_File_Undo -List [ <option>... ]",
"	%s -New_File_Undo -Help",
"",
"DESCRIPTION",
"	The %s -New_File_Undo command is used to remove new",
"	files from a change (reverse the actions of the '%s",
"	-New_File' command).",
"",
"	The %s program will attempt to intuit the file names",
"	intended.  All file names are stored within %s as",
"	relative to the root of the baseline directory tree.  The",
"	development directory and the integration directory are",
"	shadows of the baseline dirdctory, and so these relative",
"	names aply there, too.	Files named on the command line",
"	are first converted to absolute paths if necessary.  They",
"	are then compared with the baseline path, and the",
"	development directory path, and the integration directory",
"	path, to determine a root-relative name.  It is an error",
"	if the file named is outside one of these directory",
"	trees.",
"",
"	The file is removed from the list of files in the change.",
"	The file is deleted from the development directory,",
"	unless the -Keep option is used.",
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
"	csh%%	alias aenfu '%s -nfu \\!$ -v'",
"	sh$	aenfu(){%s -nfu $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in the",
"	'being_developed' state.",
"	It is an error if the change is not assigned to the",
"	current user.",
"	It is an error if the file is not in the change.",
"	It is an error if the file was not added to the change",
"	with the '%s -New_File' command.",
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

	help(text, SIZEOF(text), new_file_undo_usage);
}


static void new_file_undo_list _((void));

static void
new_file_undo_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("new_file_undo_list()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_file_undo_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				new_file_undo_usage();
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
				new_file_undo_usage();
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


static void new_file_undo_main _((void));

static void
new_file_undo_main()
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
	string_ty	*dd;
	string_ty	*bl;

	trace(("new_file_undo_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	wl_zero(&wl);
	keep = 0;
	nolog = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_file_undo_usage);
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
				new_file_undo_usage();
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
				new_file_undo_usage();
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
	 * 1. is already part of the change
	 * 2. is being created by this change
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		cstate_src	src_data;

		s1 = wl.wl_word[j];
		src_data = change_src_find(cp, s1);
		if (!src_data)
			change_fatal(cp, "file \"%S\" not in change", s1);
		if
		(
			src_data->action != file_action_create
		||
			src_data->usage != file_usage_source
		)
			change_fatal(cp, "file \"%S\" is not -New_File", s1);
	}

	/*
	 * Remove each file from the development directory,
	 * if it still exists.
	 */
	if (!keep)
	{
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
		os_become_undo();
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

	/*
	 * release the locks
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
	{
		change_verbose
		(
			cp,
			"file \"%S\" new file removed",
			wl.wl_word[j]
		);
	}
	wl_free(&wl);
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
new_file_undo()
{
	trace(("new_file_undo()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		new_file_undo_main();
		break;

	case arglex_token_help:
		new_file_undo_help();
		break;

	case arglex_token_list:
		new_file_undo_list();
		break;
	}
	trace((/*{*/"}\n"));
}
