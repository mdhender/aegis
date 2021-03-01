/*
 *	aegis - project change supervisor
 *	Copyright (C) 1993 Peter Miller.
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
 * MANIFEST: functions to manipulate mvs
 */

#include <stdio.h>

#include <ael.h>
#include <aemv.h>
#include <arglex2.h>
#include <change.h>
#include <commit.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <user.h>
#include <word.h>


static void move_file_usage _((void));

static void
move_file_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf
	(
		stderr,
	      "usage: %s -MoVe_file [ <option>... ] <old-name> <new-name>...\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -MoVe_file -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -MoVe_file -Help\n", progname);
	quit(1);
}


static void move_file_help _((void));

static void
move_file_help()
{
	static char *text[] =
	{
"NAME",
"	aegis -MoVe_file - copy a file into a change",
"",
"SYNOPSIS",
"	aegis -MoVe_file [ <option>... ] <old-name> <new-name>",
"	aegis -MoVe_file -List [ <option>... ]",
"	aegis -MoVe_file -Help",
"",
"DESCRIPTION",
"	The aegis -MoVe_file command is used to copy a file into",
"	a change, while changing its name at the same time.",
"",
"	The aegis program will attempt to determine the project",
"	file names from the file names given on the command line.",
"	All file names are stored within aegis projects as",
"	relative to the root of the baseline directory tree.  The",
"	development directory and the integration directory are",
"	shadows of this baseline directory, and so these relative",
"	names apply here, too.  Files named on the command line",
"	are first converted to absolute paths if necessary.  They",
"	are then compared with the baseline path, the development",
"	directory path, and the integration directory path, to",
"	determine a baseline-relative name.  It is an error if",
"	the file named is outside one of these directory trees.",
"",
"	The named files will be copied from the baseline (old-",
"	file) into the development directory (new-file), and",
"	added to the list of files in the change.",
"",
"	This command will cancel any build or test registrations,",
"	because adding another file logically invalidates them.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Change <number>",
"		This option may be used to specify a particular",
"		change within a project.  When no -Change option",
"		is specified, the AEGIS_CHANGE environment",
"		variable is consulted.  If that does not exist,",
"		the user's $HOME/.aegisrc file is examined for a",
"		default change field (see aeuconf(5) for more",
"		information).  If that does not exist, when the",
"		user is only working on one change within a",
"		project, that is the default change number.",
"		Otherwise, it is an error.",
"",
"	-Help",
"		This option may be used to obtain more",
"		information about how to use the aegis program.",
"",
"	-List",
"		This option may be used to obtain a list of",
"		suitable subjects for this command.  The list may",
"		be more general than expected.",
"",
"	-Not_Logging",
"		This option may be used to disable the automatic",
"		logging of output and errors to a file.  This is",
"		often useful when several aegis commands are",
"		combined in a shell script.",
"",
"	-Project <name>",
"		This option may be used to select the project of",
"		interest.  When no -Project option is specified,",
"		the AEGIS_PROJECT environment variable is",
"		consulted.  If that does not exist, the user's",
"		$HOME/.aegisrc file is examined for a default",
"		project field (see aeuconf(5) for more",
"		information).  If that does not exist, when the",
"		user is only working on changes within a single",
"		project, the project name defaults to that",
"		project.  Otherwise, it is an error.",
"",
"	-TERse",
"		This option may be used to cause listings to",
"		produce the bare minimum of information.  It is",
"		usually useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause aegis to produce",
"		more output.  By default aegis only produces",
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
"",
"RECOMMENDED ALIAS",
"	The recommended alias for this command is",
"	csh%%	alias aemv 'aegis -mv \\!* -v'",
"	sh$	aemv(){aegis -mv $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in the being",
"	developed state.",
"	It is an error if the change is not assigned to the",
"	current user.",
"	It is an error if either file is already in the change.",
"",
"EXIT STATUS",
"	The aegis command will exit with a status of 1 on any",
"	error.  The aegis command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
	};

	help(text, SIZEOF(text), move_file_usage);
}


static void move_file_list _((void));

static void
move_file_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("move_file_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(move_file_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				move_file_usage();
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
				move_file_usage();
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


static void move_file_main _((void));

static void
move_file_main()
{
	string_ty	*dd;
	string_ty	*bl;
	string_ty	*old_name;
	string_ty	*new_name;
	string_ty	*s1;
	string_ty	*s2;
	cstate		cstate_data;
	pstate		pstate_data;
	wlist		wl;
	pconf		pconf_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;
	string_ty	*config_name;
	string_ty	*from;
	string_ty	*to;
	cstate_src	c_src_data;
	pstate_src	p_src_data;

	trace(("move_file_main()\n{\n"/*}*/));
	old_name = 0;
	new_name = 0;
	project_name = 0;
	change_number = 0;
	nolog = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(move_file_usage);
			continue;

		case arglex_token_string:
			s1 = str_from_c(arglex_value.alv_string);
			os_become_orig();
			s2 = os_pathname(s1, 1);
			os_become_undo();
			str_free(s1);
			if (!old_name)
				old_name = s2;
			else if (!new_name)
				new_name = s2;
			else
				fatal("too many file names");
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				move_file_usage();
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				fatal("duplicate -Change option");
			change_number = arglex_value.alv_number;
			if (change_number < 1)
				fatal("change %ld out of range", change_number);
			break;

		case arglex_token_project:
			if (project_name)
				goto duplicate;
			if (arglex() != arglex_token_string)
				move_file_usage();
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (nolog)
			{
				duplicate:
				fatal
				(
					"duplicate %s option",
					arglex_value.alv_string
				);
			}
			nolog = 1;
			break;
		}
		arglex();
	}
	if (!old_name || !new_name)
		move_file_usage();

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
	 * 1. the absolute path of the file name is obtained
	 * 2. if the file is inside the development directory, ok
	 * 3. if the file is inside the baseline, ok
	 * 4. if neither, error
	 *
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	config_name = str_from_c(THE_CONFIG_FILE);
	if
	(
		str_equal(old_name, config_name)
	||
		str_equal(new_name, config_name)
	)
		fatal("may not rename the \"%s\" file", THE_CONFIG_FILE);
	str_free(config_name);

	dd = change_development_directory_get(cp, 1);
	bl = project_baseline_path_get(pp, 1);

	os_become_orig();
	assert(old_name->str_text[0] == '/');
	s2 = os_below_dir(dd, old_name);
	if (!s2)
		s2 = os_below_dir(bl, old_name);
	if (!s2)
		change_fatal(cp, "path \"%S\" unrelated", old_name);
	str_free(old_name);
	old_name = s2;

	assert(new_name->str_text[0] == '/');
	s2 = os_below_dir(dd, new_name);
	if (!s2)
		s2 = os_below_dir(bl, new_name);
	if (!s2)
		change_fatal(cp, "path \"%S\" unrelated", new_name);
	str_free(new_name);
	new_name = s2;
	os_become_undo();

	/*
	 * ensure that the old file
	 * 1. is not already part of the change
	 * 2. is in the baseline
	 * 3. is not a test
	 */
	if (change_src_find(cp, old_name))
		change_fatal(cp, "file \"%S\" already in change", old_name);
	p_src_data = project_src_find(pp, old_name);
	if
	(
		!p_src_data
	||
		p_src_data->about_to_be_created_by
	||
		p_src_data->deleted_by
	)
		project_fatal(pp, "file \"%S\" unknown", old_name);
	if
	(
		p_src_data->usage == file_usage_test
	||
		p_src_data->usage == file_usage_manual_test
	)
		project_fatal(pp, "may not move tests");

	/*
	 * ensure that the new file
	 * 1. is not already part of the change
	 * 2. is not in the baseline
	 */
	if (change_src_find(cp, new_name))
		change_fatal(cp, "file \"%S\" already in change", new_name);
	p_src_data = project_src_find(pp, new_name);
	if
	(
		p_src_data
	&&
		!p_src_data->about_to_be_created_by
	&&
		!p_src_data->deleted_by
	)
	{
		project_fatal
		(
			pp,
			"file \"%S\" is already in the baseline",
			new_name
		);
	}

	/*
	 * Copy the file into the development directory.
	 * Create any necessary directories along the way.
	 */
	from = str_format("%S/%S", bl, old_name);
	to = str_format("%S/%S", dd, new_name);
	user_become(up);
	os_mkdir_between(dd, new_name, 02755);
	copy_whole_file(from, to, 0);
	user_become_undo();
	str_free(from);
	str_free(to);

	/*
	 * Add the files to the change
	 */
	p_src_data = project_src_find(pp, old_name);
	assert(p_src_data);
	c_src_data = change_src_new(cp);
	c_src_data->file_name = str_copy(old_name);
	c_src_data->action = file_action_remove;
	c_src_data->usage = p_src_data->usage;
	c_src_data->move = str_copy(new_name);
	c_src_data->edit_number = str_copy(p_src_data->edit_number);

	c_src_data = change_src_new(cp);
	c_src_data->file_name = str_copy(new_name);
	c_src_data->action = file_action_create;
	c_src_data->usage = p_src_data->usage;
	c_src_data->move = str_copy(old_name);

	/*
	 * the number of files changed,
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
	change_verbose(cp, "file \"%S\" moving to \"%S\"", old_name, new_name);

	/*
	 * run the change file command
	 */
	if (!nolog)
		log_open(change_logfile_get(cp), up);
	wl_zero(&wl);
	wl_append(&wl, old_name);
	wl_append(&wl, new_name);
	change_run_change_file_command(cp, &wl, up);
	wl_free(&wl);
	str_free(old_name);
	str_free(new_name);
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
move_file()
{
	trace(("move_file()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		move_file_main();
		break;

	case arglex_token_help:
		move_file_help();
		break;

	case arglex_token_list:
		move_file_list();
		break;
	}
	trace((/*{*/"}\n"));
}
