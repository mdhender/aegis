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
 * MANIFEST: difference a change
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <arglex2.h>
#include <change.h>
#include <col.h>
#include <commit.h>
#include <diff.h>
#include <error.h>
#include <help.h>
#include <list.h>
#include <lock.h>
#include <log.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <word.h>


static void difference_usage _((void));

static void
difference_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -DIFFerence [ <option>... ][ <filename>... ]\n", progname);
	fprintf(stderr, "       %s -DIFFerence -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -DIFFerence -Help\n", progname);
	quit(1);
}


static void difference_help _((void));

static void
difference_help()
{
	static char *text[] =
	{
"NAME",
"	%s -DIFFerence - find differences between development",
"	directory and baseline",
"",
"SYNOPSIS",
"	%s -DIFFerence [ <option>... ][ <filename>... ]",
"	%s -DIFFerence -List [ <option>... ]",
"	%s -DIFFerence -Help",
"",
"DESCRIPTION",
"	The %s -DIFFerence command is used to find differences",
"	between the development directory and the baseline.  The",
"	differences will be in files with a \",D\" suffix.  The",
"	command used to perfom the differences is in the project",
"	config file (see aepconf(5) for more information).",
"",
"	If no files are named all files in the change, which were",
"	added to the change using the %s -CoPy_file command,",
"	will be differenced.",
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
"OPTIONS",
"	The following options are understood:",
"",
"	-ANticipate <change-number>",
"		This option is used to nominate a source for the",
"		reference files, rather than the baseline.  This",
"		may be used to synchronize with a change without",
"		having to wait for it to arrive in the baseline.",
"		It is an error if the anticipated change is not",
"		in the 'awaiting_integration' state.",
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
"	csh%%	alias aed '%s -diff \\!* -v'",
"	sh$	aed(){%s -diff $* -v}",
"",
"ERRORS",
"	It is an error if",
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

	help(text, SIZEOF(text), difference_usage);
}


static void difference_list _((void));

static void
difference_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("difference_list()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(difference_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				difference_usage();
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
				difference_usage();
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


static void difference_main _((void));

static void
difference_main()
{
	string_ty	*dd;
	string_ty	*bl;
	wlist		wl;
	string_ty	*s1 = 0;
	string_ty	*s2;
	wlist		need_new_build;
	pstate		pstate_data;
	pconf		pconf_data;
	cstate		cstate_data;
	size_t		j;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;

	trace(("difference_main()\n{\n"/*}*/));
	wl_zero(&wl);
	wl_zero(&need_new_build);
	project_name = 0;
	change_number = 0;
	nolog = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(difference_usage);
			continue;

		case arglex_token_string:
			s1 = str_from_c(arglex_value.alv_string);
			os_become_orig();
			s2 = os_pathname(s1, 1);
			os_become_undo();
			if (wl_member(&wl, s2))
			{
				fatal
				(
					"file \"%s\" named more than once",
					arglex_value.alv_string
				);
			}
			wl_append(&wl, s2);
			str_free(s1);
			str_free(s2);
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				difference_usage();
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
				difference_usage();
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
	 * lock on the appropriate row of the change table.
	 */
	change_cstate_lock_prepare(cp);
	lock_take();

	/*
	 * It is an error if the change is not in the in-development state.
	 * It is an error if the change is not assigned to the current user.
	 */
	pstate_data = project_pstate_get(pp);
	cstate_data = change_cstate_get(cp);
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
	 * if no files were named on the command line,
	 * then diff every file in the change.
	 */
	if (!wl.wl_nwords)
	{
		assert(cstate_data->src);
		if (!cstate_data->src->length)
			change_fatal(cp, "no files");
		for (j = 0; j < cstate_data->src->length; ++j)
		{
			cstate_src	src_data;

			src_data = cstate_data->src->list[j];
			wl_append(&wl, src_data->file_name);
		}
	}
	else
	{
		/*
		 * resolve the path of each file
		 * 1.	the absolute path of the file name is obtained
		 * 2.	if the file is inside the development directory, ok
		 * 3.	if the file is inside the baseline, ok
		 * 4.	if neither, error
		 */
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
		}

		/*
		 * confirm that each file is in the change
		 */
		for (j = 0; j < wl.wl_nwords; ++j)
		{
			s1 = wl.wl_word[j];
			if (!change_src_find(cp, s1))
			{
				change_fatal
				(
					cp,
				       "file \"%S\" is not part of this change",
					s1
				);
			}
		}
	}

	/*
	 * diff each file
	 */
	if (!nolog)
		log_open(change_logfile_get(cp), up);
	pconf_data = change_pconf_get(cp);
	dd = change_development_directory_get(cp, 1);
	bl = project_baseline_path_get(pp, 1);
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		cstate_src	src1_data;
		pstate_src	src2_data;
		string_ty	*original;
		string_ty	*input;
		string_ty	*curfile;
		string_ty	*outname;
		int		ignore;

		s1 = wl.wl_word[j];
		curfile = str_format("%S/%S", dd, s1);
		trace_string(curfile->str_text);
		outname = str_format("%S,D", curfile);
		trace_string(outname->str_text);

		/*
		 * find the relevant change src data
		 */
		src1_data = change_src_find(cp, s1);
		assert(src1_data);

		switch (src1_data->action)
		{
		case file_action_create:
			/*
			 * check if someone created it ahead of you
			 */
			src2_data = project_src_find(pp, s1);
			if (src2_data)
			{
				change_fatal
				(
					cp,
					"file \"%S\" already in baseline",
					s1
				);
			}

			/*
			 * do nothing if we can
			 */
			user_become(up);
			ignore =
				(
					src1_data->diff_time
				&&
					src1_data->diff_file_time
				&&
					os_exists(curfile)
				&&
					src1_data->diff_time == os_mtime(curfile)
				&&
					os_exists(outname)
				&&
					src1_data->diff_file_time == os_mtime(outname)
				);
			user_become_undo();
			if (ignore)
				break;

			/*
			 * difference the file
			 * from nothing
			 */
			original = str_from_c("/dev/null");
			change_run_diff_command
			(
				cp,
				up,
				original,
				curfile,
				outname
			);
			str_free(original);
			user_become(up);
			src1_data->diff_time = os_mtime(curfile);
			src1_data->diff_file_time = os_mtime(outname);
			user_become_undo();
			break;

		case file_action_remove:
			/*
			 * check if someone deleted it ahead of you
			 */
			src2_data = project_src_find(pp, s1);
			if (!src2_data)
			{
				change_fatal
				(
					cp,
					"file \"%S\" no longer in baseline",
					s1
				);
			}

			/*
			 * do nothing if we can
			 */
			user_become(up);
			ignore =
				(
					src1_data->diff_file_time
				&&
					os_exists(outname)
				&&
					src1_data->diff_file_time == os_mtime(outname)
				);
			user_become_undo();
			if (ignore)
				break;


			/*
			 * create directory for diff file
			 */
			user_become(up);
			os_mkdir_between(dd, s1, 02755);
			user_become_undo();

			/*
			 * difference the file
			 * to nothing
			 */
			original = str_format("%S/%S", bl, s1);
			input = str_from_c("/dev/null");
			change_run_diff_command
			(
				cp,
				up,
				original,
				input,
				outname
			);
			str_free(original);
			str_free(input);
			user_become(up);
			src1_data->diff_time = 0;
			src1_data->diff_file_time = os_mtime(outname);
			user_become_undo();
			break;

		case file_action_modify:
			/*
			 * find the relevant baseline src data
			 * note that someone may have deleted it from under you
			 */
			src2_data = project_src_find(pp, s1);
			if (!src2_data)
			{
				change_fatal
				(
					cp,
					"file \"%S\" no longer in baseline",
					s1
				);
			}
			if (str_equal(src1_data->edit_number, src2_data->edit_number))
			{
				/*
				 * do nothing if we can
				 */
				user_become(up);
				ignore =
					(
						src1_data->diff_time
					&&
						src1_data->diff_file_time
					&&
						os_exists(curfile)
					&&
						src1_data->diff_time == os_mtime(curfile)
					&&
						os_exists(outname)
					&&
						src1_data->diff_file_time == os_mtime(outname)
					);
				user_become_undo();
				if (ignore)
					break;

				/*
				 * If the edit number of the file in the change
				 * files table matches the edit number in the
				 * baseline files table,
				 * use the diff-command
				 */
				original = str_format("%S/%S", bl, s1);
				change_run_diff_command
				(
					cp,
					up,
					original,
					curfile,
					outname
				);
				str_free(original);
				user_become(up);
				src1_data->diff_time = os_mtime(curfile);
				src1_data->diff_file_time = os_mtime(outname);
				user_become_undo();
			}
			else
			{
				string_ty	*most_recent;

				/*
				 * name for temp file
				 */
				original =
					str_format
					(
						"/tmp/%s.%d",
						option_get_progname(),
						getpid()
					);
				user_become(up);
				undo_unlink_errok(original);
				user_become_undo();

				/*
				 * get the version out of history
				 */
				change_run_history_get_command
				(
					cp,
					s1,
					src1_data->edit_number,
					original,
					up
				);

				/*
				 * use the diff3-command
				 */
				most_recent = str_format("%S/%S", bl, s1);
				change_run_diff3_command
				(
					cp,
					up,
					original,
					most_recent,
					curfile,
					outname
				);
				str_free(most_recent);

				/*
				 * Remember to remove the temporary file when
				 * finished.
				 */
				user_become(up);
				os_unlink(original);
				user_become_undo();
				str_free(original);

				/*
				 * Set the edit number in the change
				 * to the latest baseline edit number.
				 */
				str_free(src1_data->edit_number);
				src1_data->edit_number =
					str_copy(src2_data->edit_number);

				/*
				 * Invalidate the build-time field.
				 * Invalidate the diff-time fields.
				 * (because need new build)
				 */
				wl_append(&need_new_build, src1_data->file_name);
				cstate_data->build_time = 0;
				src1_data->diff_time = 0;
				src1_data->diff_file_time = 0;
			}
			break;
		}
		str_free(curfile);
		str_free(outname);
	}

	/*
	 * If the change row (or change file table) changed,
	 * write it out.
	 * Release advisory lock.
	 */
	change_cstate_write(cp);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	if (wl.wl_nwords == cstate_data->src->length)
		change_verbose(cp, "difference complete");
	else
		change_verbose(cp, "partial difference done");
	if (need_new_build.wl_nwords)
	{
		for (j = 0; j < need_new_build.wl_nwords; ++j)
		{
			change_error
			(
				cp,
			"file \"%S\" was out of date, see \"%S,D\" for details",
				need_new_build.wl_word[j],
				need_new_build.wl_word[j]
			);
		}
		error("new '%s -Build' required", option_get_progname());
	}
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
difference()
{
	trace(("difference()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		difference_main();
		break;

	case arglex_token_help:
		difference_help();
		break;

	case arglex_token_list:
		difference_list();
		break;
	}
	trace((/*{*/"}\n"));
}
