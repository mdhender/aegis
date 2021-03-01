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

#include <aecp.h>
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


static void copy_file_usage _((void));

static void
copy_file_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf
	(
		stderr,
		"usage: %s -CoPy_file [ <option>... ] <filename>...\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -CoPy_file -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -CoPy_file -Help\n", progname);
	quit(1);
}


static void copy_file_help _((void));

static void
copy_file_help()
{
	static char *text[] =
	{
"NAME",
"	%s -CoPy_file - copy a file into a change",
"",
"SYNOPSIS",
"	%s -CoPy_file [ <option>... ] <filename>...",
"	%s -CoPy_file -List [ <option>... ]",
"	%s -CoPy_file -Help",
"",
"DESCRIPTION",
"	The %s -CoPy_file command is used to copy a file into",
"	a change.",
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
"	The named files will be copied from the baseline into the",
"	development directory, and added to the list of files in",
"	the change.",
"",
"	This command will cancel any build or test registrations,",
"	because adding another file logically invalidates them.  If",
"	the config file was added, any diff registration will also",
"	be cancelled.",
"",
"	This command may be used to copy tests into a change, not",
"	just source files.  Tests are treated just like	any other",
"	source file, and are subject to	the same process.",
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
"	-Delta <number>",
"		This optioon may be used to specify a particular",
"		delta in the project's history to copy the file",
"		from, rather than the most current version.  It is",
"		an error if the delta specified does not exist.",
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
"	-Output <filename>",
"		This option may be used to specify an output file of",
"		a file being copied from the baseline.  Only one",
"		baseline file may be named when this option is used.",
"		The file name \"-\" is understood to mean the standard",
"		output.",
"",
"	-OverWriting",
"		This option may be used to force overwriting of",
"		files.	The deafult action is to give and error",
"		if an existing file would be overwritten.",
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
"	csh%%	alias aecp '%s -cp \\!* -v'",
"	sh$	aecp(){%s -cp $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in the",
"	'being_developed' state.",
"	It is an error if the change is not assigned to the",
"	current user.",
"	It is an error if the file is already in the change and",
"	the -OverWrite option is not specified.",
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

	help(text, SIZEOF(text), copy_file_usage);
}


static void copy_file_list _((void));

static void
copy_file_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("copy_file_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(copy_file_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				copy_file_usage();
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
				copy_file_usage();
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


static void copy_file_main _((void));

static void
copy_file_main()
{
	string_ty	*id;
	string_ty	*dd;
	string_ty	*bl;
	wlist		wl;
	string_ty	*s1;
	string_ty	*s2;
	int		stomp;
	cstate		cstate_data;
	pstate		pstate_data;
	int		j;
	pconf		pconf_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;
	char		*output;
	long		delta_number;
	string_ty	*delta_number_implies_edit_number = 0;
	int		config_seen;
	string_ty	*config_name;

	trace(("copy_file_main()\n{\n"/*}*/));
	wl_zero(&wl);
	stomp = 0;
	project_name = 0;
	change_number = 0;
	nolog = 0;
	output = 0;
	delta_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(copy_file_usage);
			continue;

		case arglex_token_overwriting:
			if (stomp)
			{
				duplicate:
				fatal
				(
					"duplicate %s option",
					arglex_value.alv_string
				);
			}
			stomp = 1;
			break;

		case arglex_token_string:
			s1 = str_from_c(arglex_value.alv_string);
			os_become_orig();
			s2 = os_pathname(s1, 1);
			os_become_undo();
			str_free(s1);
			if (wl_member(&wl, s2))
			{
				fatal
				(
					"file \"%s\" named more then once",
					s2->str_text
				);
			}
			wl_append(&wl, s2);
			str_free(s2);
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				copy_file_usage();
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
				copy_file_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (nolog)
				goto duplicate;
			nolog = 1;
			break;

		case arglex_token_delta:
			if (delta_number)
				goto duplicate;
			if (arglex() != arglex_token_number)
				copy_file_usage();
			delta_number = arglex_value.alv_number;
			if (delta_number < 1)
			{
				fatal
				(
					"delta %ld is out of range",
					delta_number
				);
			}
			break;

		case arglex_token_output:
			if (output)
				goto duplicate;
			switch (arglex())
			{
			default:
				copy_file_usage();

			case arglex_token_stdio:
				output = "";
				break;

			case arglex_token_string:
				output = arglex_value.alv_string;
				break;
			}
			break;
		}
		arglex();
	}
	if (!wl.wl_nwords)
		fatal("no files named");

	/*
	 * make sure output is unambiguous
	 */
	if (output)
	{
		if (wl.wl_nwords != 1)
		{
			fatal
			(
		"only one file may be specifed when the -Output option is used"
			);
		}
		stomp = 1;
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
	 * lock the change file
	 */
	if (!output)
	{
		change_cstate_lock_prepare(cp);
		lock_take();
	}
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);
	pconf_data = change_pconf_get(cp);

	/*
	 * When there is no explicit output file:
	 * It is an error if the change is not in the in_development state.
	 * It is an error if the change is not assigned to the current user.
	 */
	if (output)
	{
		switch (cstate_data->state)
		{
		case cstate_state_being_developed:
		case cstate_state_being_reviewed:
		case cstate_state_awaiting_integration:
		case cstate_state_being_integrated:
			break;

		default:
			change_fatal(cp, "not in a suitable state");
		}
	}
	else
	{
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
	}

	/*
	 * it is an error if the delta does not exist
	 */
	if (delta_number)
	{
		switch (project_delta_exists(pp, delta_number))
		{
		case 0:
			project_fatal(pp, "delta %ld unknown", delta_number);

		case -1:
			/*
			 * the delta given was the current baseline,
			 * don't bother with history
			 *
			 * Except when integrating,
			 * in which case the integration copy may differ.
			 */
			if (cstate_data->state != cstate_state_being_integrated)
				delta_number = 0;
			break;
		}
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
	config_seen = 0;
	config_name = str_from_c(THE_CONFIG_FILE);
	dd = change_development_directory_get(cp, 1);
	if (cstate_data->state == cstate_state_being_integrated)
		id = change_integration_directory_get(cp, 1);
	else
		id = 0;
	bl = project_baseline_path_get(pp, 1);
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		s1 = wl.wl_word[j];
		assert(s1->str_text[0] == '/');
		s2 = os_below_dir(dd, s1);
		if (!s2)
			s2 = os_below_dir(bl, s1);
		if (!s2 && id)
			s2 = os_below_dir(id, s1);
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
	 * 1. is not already part of the change
	 * 2. is in the baseline
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		pstate_src	src_data;

		s1 = wl.wl_word[j];
		if (change_src_find(cp, s1) && !stomp && !output)
			change_fatal(cp, "file \"%S\": will not overwrite", s1);
		if (output)
		{
			cstate_src	c_src_data;

			/*
			 * OK to use a file that "almost" exists
			 */
			c_src_data = change_src_find(cp, s1);
			if
			(
				c_src_data
			&&
				c_src_data->action == file_action_create
			)
				continue;
		}
		src_data = project_src_find(pp, s1);
		if
		(
			!src_data
		||
			src_data->about_to_be_created_by
		||
			src_data->deleted_by
		)
			project_fatal(pp, "file \"%S\" unknown", s1);
		if (src_data && src_data->usage == file_usage_build && !output)
		{
			change_fatal
			(
				cp,
				"file \"%S\" is built, may not copy",
				s1
			);
		}
	}

	/*
	 * Copy each file into the development directory.
	 * Create any necessary directories along the way.
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		string_ty	*from;
		string_ty	*to;

		s1 = wl.wl_word[j];
		if (delta_number)
		{
			/*
			 * find the edit number,
			 * given fie file name and delta number
			 *
			 * NULL returned means that the file
			 * does not exist at the given delta.
			 */
			delta_number_implies_edit_number =
				project_delta_to_edit(pp, delta_number, s1);
			if (delta_number_implies_edit_number)
			{
				/*
				 * make a temporary file
				 */
				from = os_edit_filename();
				user_become(up);
				undo_unlink_errok(from);
				user_become_undo();

				/* 
				 * get the file from history
				 */
				change_run_history_get_command
				(
					cp,
					s1,
					delta_number_implies_edit_number,
					from,
					up
				);
			}
			else
				from = str_from_c("/dev/null");

			/*
			 * figure where to send it
			 */
			if (output)
				to = str_from_c(output);
			else
				to = str_format("%S/%S", dd, s1);

			/*
			 * copy the file
			 */
			user_become(up);
			if (!output)
				os_mkdir_between(dd, s1, 02755);
			copy_whole_file(from, to, 0);

			/*
			 * clean up afterwards
			 */
			if (delta_number_implies_edit_number)
				os_unlink_errok(from);
			user_become_undo();
			str_free(from);
			str_free(to);
		}
		else
		{
			if (id)
				from = str_format("%S/%S", id, s1);
			else
				from = str_format("%S/%S", bl, s1);
			if (output)
				to = str_from_c(output);
			else
				to = str_format("%S/%S", dd, s1);

			/*
			 * copy the file
			 */
			user_become(up);
			if (!output)
				os_mkdir_between(dd, s1, 02755);
			copy_whole_file(from, to, 0);
			user_become_undo();

			/* 
			 * clean up afterwards
			 */
			str_free(from);
			str_free(to);
		}
	}

	/*
	 * Add each file to the change file,
	 * or update the edit number.
	 */
	if (!output)
	{
		for (j = 0; j < wl.wl_nwords; ++j)
		{
			cstate_src	c_src_data;
			pstate_src	p_src_data;
	
			s1 = wl.wl_word[j];
			p_src_data = project_src_find(pp, s1);
			assert(p_src_data);
			c_src_data = change_src_find(cp, s1);
			if (!c_src_data)
			{
				c_src_data = change_src_new(cp);
				c_src_data->file_name = str_copy(s1);
				c_src_data->action = file_action_modify;
				c_src_data->usage = p_src_data->usage;
	
				/*
				 * The change now has at least one test,
				 * so cancel any testing exemption.
				 * (But test_baseline_exempt is still viable.)
				 */
				if
				(
					c_src_data->usage == file_usage_test 
				||
					c_src_data->usage == file_usage_manual_test
				)
					cstate_data->test_exempt = 0;
			}
			else
				str_free(c_src_data->edit_number);
			assert(p_src_data->edit_number);
			if (delta_number_implies_edit_number)
				c_src_data->edit_number =
					delta_number_implies_edit_number;
			else
				c_src_data->edit_number =
					str_copy(p_src_data->edit_number);
			if (config_seen)
				c_src_data->diff_time = 0;
		}

		/*
		 * the number of files changed,
		 * so stomp on the validation fields.
		 */
		cstate_data->build_time = 0;
		cstate_data->test_time = 0;
		cstate_data->test_baseline_time = 0;
		cstate_data->regression_test_time = 0;
	}

	/*
	 * release the locks
	 */
	if (!output)
	{
		change_cstate_write(cp);
		commit();
		lock_release();
	}

	/*
	 * verbose success message
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
		change_verbose(cp, "file \"%S\" copied", wl.wl_word[j]);

	/*
	 * run the change file command
	 */
	if (!output)
	{
		if (!nolog)
			log_open(change_logfile_get(cp), up);
		change_run_change_file_command(cp, &wl, up);
	}
	wl_free(&wl);
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
copy_file()
{
	trace(("copy_file()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		copy_file_main();
		break;

	case arglex_token_help:
		copy_file_help();
		break;

	case arglex_token_list:
		copy_file_list();
		break;
	}
	trace((/*{*/"}\n"));
}
