/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999 Peter Miller;
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
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to implement new file undo
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>

#include <ael/change/files.h>
#include <aenfu.h>
#include <arglex2.h>
#include <change_bran.h>
#include <change/file.h>
#include <col.h>
#include <commit.h>
#include <error.h>
#include <glue.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <str_list.h>


static void new_file_undo_usage _((void));

static void
new_file_undo_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf
	(
		stderr,
		"usage: %s -New_File_Undo <filename>... [ <option>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -New_File_Undo -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -New_File_Undo -Help\n", progname);
	quit(1);
}


static void new_file_undo_help _((void));

static void
new_file_undo_help()
{
	help("aenfu", new_file_undo_usage);
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
				option_needs_number(arglex_token_change, new_file_undo_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, new_file_undo_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "Number", "%ld", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, new_file_undo_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, new_file_undo_usage);
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
	string_list_ty	wl, wl2;
	cstate		cstate_data;
	size_t		j, k;
	string_ty	*s1;
	string_ty	*s2;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	log_style_ty	log_style;
	user_ty		*up;
	int		number_of_errors;
	string_list_ty	search_path;
	int		based;
	string_ty	*base;

	trace(("new_file_undo_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	string_list_constructor(&wl);
	log_style = log_style_append_default;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_file_undo_usage);
			continue;

		case arglex_token_file:
		case arglex_token_directory:
			if (arglex() == arglex_token_string)
				new_file_undo_usage();
			/* fall through... */

		case arglex_token_string:
			s2 = str_from_c(arglex_value.alv_string);
			string_list_append(&wl, s2);
			str_free(s2);
			break;

		case arglex_token_keep:
		case arglex_token_interactive:
		case arglex_token_no_keep:
			user_delete_file_argument(new_file_undo_usage);
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, new_file_undo_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, new_file_undo_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "Number", "%ld", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, new_file_undo_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, new_file_undo_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (log_style == log_style_none)
				duplicate_option(new_file_undo_usage);
			log_style = log_style_none;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(new_file_undo_usage);
			break;

		case arglex_token_base_relative:
		case arglex_token_current_relative:
			user_relative_filename_preference_argument(new_file_undo_usage);
			break;
		}
		arglex();
	}
	if (!wl.nstrings)
	{
		error_intl(0, i18n("no file names"));
		new_file_undo_usage();
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
	change_cstate_lock_prepare(cp);
	lock_take();
	cstate_data = change_cstate_get(cp);

	log_open(change_logfile_get(cp), up, log_style);

	/*
	 * It is an error if the change is not in the in_development state.
	 * It is an error if the change is not assigned to the current user.
	 */
	if (cstate_data->state != cstate_state_being_developed)
		change_fatal(cp, 0, i18n("bad nfu state"));
	if (change_is_a_branch(cp))
		change_fatal(cp, 0, i18n("bad nfu branch"));
	if (!str_equal(change_developer_name(cp), user_name(up)))
		change_fatal(cp, 0, i18n("not developer"));

	/*
	 * Search path to resolve filenames
	 */
	change_search_path_get(cp, &search_path, 1);

	/*
	 * Find the base for relative filenames.
	 */
	based =
		(
			search_path.nstrings >= 1
		&&
			(
				user_relative_filename_preference
				(
					up,
				      uconf_relative_filename_preference_current
				)
			==
				uconf_relative_filename_preference_base
			)
		);
	if (based)
		base = search_path.string[0];
	else
	{
		os_become_orig();
		base = os_curdir();
		os_become_undo();
	}

	/*
	 * resolve the path of each file
	 * 1.	the absolute path of the file name is obtained
	 * 2.	if the file is inside the development directory, ok
	 * 3.	if the file is inside the baseline, ok
	 * 4.	if neither, error
	 */
	number_of_errors = 0;
	string_list_constructor(&wl2);
	for (j = 0; j < wl.nstrings; ++j)
	{
		string_list_ty	wl_in;

		s1 = wl.string[j];
		if (s1->str_text[0] == '/')
			s2 = str_copy(s1);
		else
			s2 = str_format("%S/%S", base, s1);
		user_become(up);
		s1 = os_pathname(s2, 1);
		user_become_undo();
		str_free(s2);
		s2 = 0;
		for (k = 0; k < search_path.nstrings; ++k)
		{
			s2 = os_below_dir(search_path.string[k], s1);
			if (s2)
				break;
		}
		str_free(s1);
		if (!s2)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", wl.string[j]);
			change_error(cp, scp, i18n("$filename unrelated"));
			sub_context_delete(scp);
			++number_of_errors;
			continue;
		}
		change_file_directory_query(cp, s2, &wl_in, 0);
		if (wl_in.nstrings)
		{
			int	used;

			/*
			 * If the user named a directory, add all of the
			 * source files in that directory, provided they
			 * are not tests.
			 */
			used = 0;
			for (k = 0; k < wl_in.nstrings; ++k)
			{
				fstate_src	src_data;
				string_ty	*s3;

				s3 = wl_in.string[k];
				src_data = change_file_find(cp, s3);
				assert(src_data);
				if
				(
					src_data
				&&
					src_data->action == file_action_create
				&&
					(
						src_data->usage == file_usage_source
					||
						src_data->usage == file_usage_build
					)
				)
				{
					if (string_list_member(&wl2, s3))
					{
						sub_context_ty	*scp;

						scp = sub_context_new();
						sub_var_set(scp, "File_Name", "%S", s3);
						change_error(cp, scp, i18n("too many $filename"));
						sub_context_delete(scp);
						++number_of_errors;
					}
					else
						string_list_append(&wl2, s3);
					++used;
				}
			}
			if (!used)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				if (s2->str_length)
					sub_var_set(scp, "File_Name", "%S", s2);
				else
					sub_var_set(scp, "File_Name", ".");
				sub_var_set(scp, "Number", "%ld", (long)wl_in.nstrings);
				sub_var_optional(scp, "Number");
				change_error
				(
					cp,
					scp,
			  i18n("directory $filename contains no relevant files")
				);
				sub_context_delete(scp);
				++number_of_errors;
			}
		}
		else
		{
			if (string_list_member(&wl2, s2))
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", s2);
				change_error(cp, scp, i18n("too many $filename"));
				sub_context_delete(scp);
				++number_of_errors;
			}
			else
				string_list_append(&wl2, s2);
		}
		string_list_destructor(&wl_in);
		str_free(s2);
	}
	string_list_destructor(&wl);
	wl = wl2;
	string_list_destructor(&search_path);

	/*
	 * ensure that each file
	 * 1. is already part of the change
	 * 2. is being created by this change
	 */
	for (j = 0; j < wl.nstrings; ++j)
	{
		fstate_src	src_data;

		s1 = wl.string[j];
		src_data = change_file_find(cp, s1);
		if (!src_data)
		{
			src_data = change_file_find_fuzzy(cp, s1);
			if (src_data)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", s1);
				sub_var_set(scp, "Guess", "%S", src_data->file_name);
				change_error(cp, scp, i18n("no $filename, closest is $guess"));
				sub_context_delete(scp);
			}
			else
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", s1);
				change_error(cp, scp, i18n("no $filename"));
				sub_context_delete(scp);
			}
			++number_of_errors;
			continue;
		}
		if
		(
			src_data->action != file_action_create
		||
			(
				src_data->usage != file_usage_source
			&&
				src_data->usage != file_usage_build
			)
		)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", s1);
			change_error(cp, scp, i18n("bad nf undo $filename"));
			sub_context_delete(scp);
			++number_of_errors;
		}
	}
	if (number_of_errors)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Number", "%d", number_of_errors);
		sub_var_optional(scp, "Number");
		change_fatal(cp, scp, i18n("new file undo fail"));
		sub_context_delete(scp);
	}

	/*
	 * Remove each file from the development directory,
	 * if it still exists.
	 */
	for (j = 0; j < wl.nstrings; ++j)
	{
		int		exists;

		s1 = wl.string[j];
		s2 = change_file_path(cp, s1);
		user_become(up);
		exists = os_exists(s2);
		user_become_undo();
		if (exists && user_delete_file_query(up, s1, 0))
		{
			user_become(up);
			commit_unlink_errok(s2);
			user_become_undo();
		}

		s1 = str_format("%S,D", s2);
		str_free(s2);
		s2 = s1;
		user_become(up);
		if (os_exists(s2))
			commit_unlink_errok(s2);
		user_become_undo();
		str_free(s2);
	}

	/*
	 * Remove each file to the change file,
	 * and write it back out.
	 */
	for (j = 0; j < wl.nstrings; ++j)
		change_file_remove(cp, wl.string[j]);

	/*
	 * the number of files changed, or the version did,
	 * so stomp on the validation fields.
	 */
	change_build_times_clear(cp);

	/*
	 * run the change file command
	 * and the project file command if necessary
	 */
	change_run_change_file_command(cp, &wl, up);
	change_run_project_file_command(cp, up);

	/*
	 * release the locks
	 */
	change_cstate_write(cp);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	for (j = 0; j < wl.nstrings; ++j)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", wl.string[j]);
		change_verbose(cp, scp, i18n("new file undo $filename complete"));
		sub_context_delete(scp);
	}
	string_list_destructor(&wl);
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
