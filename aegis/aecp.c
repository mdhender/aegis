/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998 Peter Miller;
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
 * MANIFEST: functions to implement copy file
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>
#include <ac/libintl.h>

#include <aecp.h>
#include <ael.h>
#include <arglex2.h>
#include <commit.h>
#include <change_bran.h>
#include <change_file.h>
#include <error.h>
#include <file.h>
#include <gettime.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project_file.h>
#include <project_hist.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <str_list.h>


static void copy_file_usage _((void));

static void
copy_file_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf
	(
		stderr,
		"usage: %s -CoPy_file [ <option>... ] <filename>...\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -CoPy_file -INDependent [ <option>... ] <filename>...\n",
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
	help("aecp", copy_file_usage);
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
				option_needs_number(arglex_token_change, copy_file_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, copy_file_usage);
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
				option_needs_name(arglex_token_project, copy_file_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, copy_file_usage);
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


static void copy_file_independent _((void));

static void
copy_file_independent()
{
	string_ty	*dd;
	string_list_ty	wl;
	string_list_ty	wl2;
	string_list_ty	wl_in;
	string_list_ty	wl_out;
	string_ty	*s1;
	string_ty	*s2;
	int		j, k;
	string_ty	*project_name;
	project_ty	*pp;
	project_ty	*pp2;
	user_ty		*up;
	long		delta_number;
	time_t		delta_date;
	char		*delta_name;
	string_ty	*delta_date_implies_edit_number = 0;
	int		number_of_errors;
	string_list_ty	search_path;
	char		*branch;
	int		trunk;
	change_ty	*cp_bogus;
	int		based;
	string_ty	*base;

	trace(("copy_file_independent()\n{\n"/*}*/));
	arglex();
	string_list_constructor(&wl);
	project_name = 0;
	delta_date = (time_t)-1;
	delta_number = -1;
	delta_name = 0;
	branch = 0;
	trunk = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(copy_file_usage);
			continue;

		case arglex_token_directory:
			if (arglex() != arglex_token_string)
				option_needs_dir(arglex_token_directory, copy_file_usage);
			goto get_file_names;

		case arglex_token_file:
			if (arglex() != arglex_token_string)
				option_needs_files(arglex_token_file, copy_file_usage);
			/* fall through... */

		case arglex_token_string:
			get_file_names:
			s2 = str_from_c(arglex_value.alv_string);
			string_list_append(&wl, s2);
			str_free(s2);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, copy_file_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, copy_file_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_delta:
			if (delta_number >= 0 || delta_name)
				duplicate_option(copy_file_usage);
			switch (arglex())
			{
			default:
				option_needs_number(arglex_token_delta, copy_file_usage);
				/*NOTREACHED*/

			case arglex_token_number:
				delta_number = arglex_value.alv_number;
				if (delta_number < 0)
				{
					sub_context_ty	*scp;

					scp = sub_context_new();
					sub_var_set(scp, "Number", "%ld", delta_number);
					fatal_intl(scp, i18n("delta $number out of range"));
					/* NOTREACHED */
					sub_context_delete(scp);
				}
				break;

			case arglex_token_string:
				delta_name = arglex_value.alv_string;
				break;
			}
			break;

		case arglex_token_delta_date:
			if (delta_date != (time_t)-1)
				duplicate_option(copy_file_usage);
			if (arglex() != arglex_token_string)
			{
				option_needs_string(arglex_token_delta_date, copy_file_usage);
				/*NOTREACHED*/
			}
			delta_date = date_scan(arglex_value.alv_string);
			if (delta_date == (time_t)-1)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "Name", "%s", arglex_value.alv_string);
				fatal_intl(scp, i18n("date $name unknown"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_branch:
			if (branch)
				duplicate_option(copy_file_usage);
			switch (arglex())
			{
			default:
				option_needs_number(arglex_token_branch, copy_file_usage);

			case arglex_token_number:
			case arglex_token_string:
				branch = arglex_value.alv_string;
				break;
			}
			break;

		case arglex_token_trunk:
			if (trunk)
				duplicate_option(copy_file_usage);
			++trunk;
			break;

		case arglex_token_base_relative:
		case arglex_token_current_relative:
			user_relative_filename_preference_argument(copy_file_usage);
			break;
		}
		arglex();
	}
	if (!wl.nstrings)
	{
		error_intl(0, i18n("no file names"));
		copy_file_usage();
	}
	if (trunk)
	{
		if (branch)
		{
			mutually_exclusive_options
			(
				arglex_token_branch,
				arglex_token_trunk,
				copy_file_usage
			);
		}
		branch = "";
	}
	if ((delta_name || delta_number >= 0) && delta_date != (time_t)-1)
	{
		mutually_exclusive_options
		(
			arglex_token_delta,
			arglex_token_delta_date,
			copy_file_usage
		);
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
	 * locate which branch
	 */
	if (branch)
		pp2 = project_find_branch(pp, branch);
	else
		pp2 = pp;

	/*
	 * locate user data
	 */
	up = user_executing(pp);

	/*
	 * Take a read lock on the baseline, to ensure that it does
	 * not change (aeip) for the duration of the copy.
	 */
	project_baseline_read_lock_prepare(pp2);
	lock_take();

	/*
	 * it is an error if the delta does not exist
	 */
	trace(("mark\n"));
	if (delta_name)
	{
		s1 = str_from_c(delta_name);
		delta_number = project_history_delta_by_name(pp2, s1, 0);
		str_free(s1);
	}
	if (delta_number >= 0)
	{
		delta_date = project_history_delta_to_timestamp(pp2, delta_number);
		if (delta_date == (time_t)-1)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%ld", delta_number);
			project_fatal(pp2, scp, i18n("no delta $name"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
	}

	/*
	 * build the list of places to look
	 * when resolving the file name
	 *
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	trace(("mark\n"));
	os_become_orig();
	dd = os_curdir();
	os_become_undo();
	trace(("mark\n"));
	string_list_constructor(&search_path);
	project_search_path_get(pp2, &search_path, 1);

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
					uconf_relative_filename_preference_base
				)
			==
				uconf_relative_filename_preference_base
			)
		);
	if (based)
		base = str_copy(search_path.string[0]);
	else
		base = dd;
	trace(("mark\n"));
	string_list_prepend(&search_path, dd);

	/*
	 * resolve the path of each file
	 * 1. the absolute path of the file name is obtained
	 * 2. if the file is inside the search list
	 * 3. if neither, error
	 */
	trace(("mark\n"));
	string_list_constructor(&wl2);
	number_of_errors = 0;
	for (j = 0; j < wl.nstrings; ++j)
	{
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
			project_error(pp, scp, i18n("$filename unrelated"));
			sub_context_delete(scp);
			++number_of_errors;
			continue;
		}
		project_file_dir(pp2, s2, &wl_in, &wl_out);
		if (delta_date != (time_t)-1)
			string_list_append_list(&wl_in, &wl_out);
		if (wl_in.nstrings)
			string_list_append_list_unique(&wl2, &wl_in);
		else
			string_list_append_unique(&wl2, s2);
		string_list_destructor(&wl_in);
		string_list_destructor(&wl_out);
		str_free(s2);
	}
	string_list_destructor(&search_path);
	string_list_destructor(&wl);
	wl = wl2;

	/*
	 * ensure that each file
	 * is in the baseline
	 */
	trace(("mark\n"));
	for (j = 0; j < wl.nstrings; ++j)
	{
		fstate_src	src_data;

		s1 = wl.string[j];
		src_data = project_file_find(pp2, s1);
		if
		(
			!src_data
		||
			src_data->about_to_be_created_by
		||
			(delta_date == (time_t)-1 && src_data->deleted_by)
		)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			src_data = project_file_find_fuzzy(pp2, s1);
			sub_var_set(scp, "File_Name", "%S", s1);
			if (src_data)
			{
				sub_var_set(scp, "Guess", "%S", src_data->file_name);
				project_error
				(
					pp2,
					scp,
					i18n("no $filename, closest is $guess")
				);
			}
			else
				project_error(pp2, scp, i18n("no $filename"));
			sub_context_delete(scp);
			++number_of_errors;
			continue;
		}
		if (src_data && src_data->usage == file_usage_build)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", s1);
			project_error(pp, scp, i18n("$filename is built"));
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
		project_fatal(pp, scp, i18n("no files copied"));
		sub_context_delete(scp);
	}

	/*
	 * create a fake change,
	 * so can set environment variables
	 * for the test
	 */
	trace(("mark\n"));
	cp_bogus = change_alloc(pp, project_next_change_number(pp, 1));
	change_bind_new(cp_bogus);
	change_architecture_from_pconf(cp_bogus);
	cp_bogus->bogus = 1;

	/*
	 * Copy each file into the development directory.
	 * Create any necessary directories along the way.
	 */
	trace(("mark\n"));
	for (j = 0; j < wl.nstrings; ++j)
	{
		string_ty	*from;
		string_ty	*to;

		s1 = wl.string[j];
		if (delta_date != (time_t)-1)
		{
			/*
			 * find the edit number,
			 * given fie file name and delta number
			 *
			 * NULL returned means that the file
			 * does not exist at the given delta.
			 */
			delta_date_implies_edit_number =
				project_delta_date_to_edit(pp2, delta_date, s1);
			if (delta_date_implies_edit_number)
			{
				fstate_src	psrc_data;

				/*
				 * Look in the immediate project...
				 * the other branch may have been completed.
				 */
				psrc_data = project_file_find(pp, s1);
				assert(psrc_data);
				assert(psrc_data->edit_number);
				if
				(
					psrc_data
				&&
					psrc_data->edit_number
				&&
					str_equal(psrc_data->edit_number, delta_date_implies_edit_number)
				&&
					psrc_data->action != file_action_remove
				)
				{
					/*
					 * do a simple file copy
					 * from the immediate project
					 */
					from = project_file_path(pp, s1);
					str_free(delta_date_implies_edit_number);
					delta_date_implies_edit_number = 0;
				}
				else
				{
					/*
					 * make a temporary file
					 */
					from = os_edit_filename(0);
					os_become_orig();
					undo_unlink_errok(from);
					os_become_undo();

					/*
					 * get the file from history
					 */
					change_run_history_get_command
					(
						cp_bogus,
						s1,
						delta_date_implies_edit_number,
						from,
						up
					);
				}
			}
			else
				from = str_from_c("/dev/null");

			/*
			 * figure where to send it
			 */
			to = str_format("%S/%S", dd, s1);

			/*
			 * copy the file
			 */
			trace(("mark\n"));
			os_become_orig();
			os_mkdir_between(dd, s1, 02755);
			if (os_exists(to))
				os_unlink(to);
			copy_whole_file(from, to, 0);

			/*
			 * clean up afterwards
			 */
			if (delta_date_implies_edit_number)
			{
				os_unlink_errok(from);
				str_free(delta_date_implies_edit_number);
			}
			os_become_undo();
			str_free(from);
			str_free(to);
		}
		else
		{
			from = project_file_path(pp2, s1);
			to = str_format("%S/%S", dd, s1);

			/*
			 * copy the file
			 */
	trace(("mark\n"));
			os_become_orig();
			os_mkdir_between(dd, s1, 02755);
			if (os_exists(to))
				os_unlink(to);
			copy_whole_file(from, to, 0);
			os_become_undo();

			/*
			 * clean up afterwards
			 */
			str_free(from);
			str_free(to);
		}
	}
	change_free(cp_bogus);

	/*
	 * release the baseline lock
	 */
	trace(("mark\n"));
	lock_release();

	/*
	 * verbose success message
	 */
	for (j = 0; j < wl.nstrings; ++j)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", wl.string[j]);
		project_verbose(pp, scp, i18n("copied $filename"));
		sub_context_delete(scp);
	}

	string_list_destructor(&wl);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


static void copy_file_main _((void));

static void
copy_file_main()
{
	string_ty	*dd;
	string_list_ty	wl;
	string_list_ty	wl2;
	string_list_ty	wl_in;
	string_list_ty	wl_out;
	string_ty	*s1;
	string_ty	*s2;
	int		stomp;
	cstate		cstate_data;
	int		j, k;
	string_ty	*project_name;
	project_ty	*pp;
	project_ty	*pp2;
	long		change_number;
	change_ty	*cp;
	log_style_ty	log_style;
	user_ty		*up;
	char		*output;
	time_t		delta_date;
	long		delta_number;
	char		*delta_name;
	string_ty	*delta_date_implies_edit_number = 0;
	int		config_seen;
	string_ty	*config_name;
	int		number_of_errors;
	string_list_ty	search_path;
	char		*branch;
	int		trunk;
	int		read_only;
	int		mode;
	int		based;
	string_ty	*base;
	sub_context_ty	*scp;

	trace(("copy_file_main()\n{\n"/*}*/));
	string_list_constructor(&wl);
	stomp = 0;
	project_name = 0;
	change_number = 0;
	log_style = log_style_append_default;
	output = 0;
	delta_date = (time_t)-1;
	delta_number = -1;
	delta_name = 0;
	branch = 0;
	trunk = 0;
	read_only = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(copy_file_usage);
			continue;

		case arglex_token_overwriting:
			if (stomp)
				duplicate_option(copy_file_usage);
			stomp = 1;
			break;

		case arglex_token_directory:
			if (arglex() != arglex_token_string)
				option_needs_dir(arglex_token_directory, copy_file_usage);
			goto get_file_names;

		case arglex_token_file:
			if (arglex() != arglex_token_string)
				option_needs_files(arglex_token_file, copy_file_usage);
			/* fall through... */

		case arglex_token_string:
			get_file_names:
			s2 = str_from_c(arglex_value.alv_string);
			string_list_append(&wl, s2);
			str_free(s2);
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, copy_file_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, copy_file_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				scp = sub_context_new();
				sub_var_set(scp, "Number", "%ld", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, copy_file_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, copy_file_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (log_style == log_style_none)
				duplicate_option(copy_file_usage);
			log_style = log_style_none;
			break;

		case arglex_token_delta:
			if (delta_number >= 0 || delta_name)
				duplicate_option(copy_file_usage);
			switch (arglex())
			{
			default:
				option_needs_number(arglex_token_delta, copy_file_usage);
				/*NOTREACHED*/

			case arglex_token_number:
				delta_number = arglex_value.alv_number;
				if (delta_number < 0)
				{
					scp = sub_context_new();
					sub_var_set(scp, "Number", "%ld", delta_number);
					fatal_intl(scp, i18n("delta $number out of range"));
					/* NOTREACHED */
					sub_context_delete(scp);
				}
				break;

			case arglex_token_string:
				delta_name = arglex_value.alv_string;
				break;
			}
			break;

		case arglex_token_delta_date:
			if (delta_date != (time_t)-1)
				duplicate_option(copy_file_usage);
			if (arglex() != arglex_token_string)
			{
				option_needs_string(arglex_token_delta_date, copy_file_usage);
				/*NOTREACHED*/
			}
			delta_date = date_scan(arglex_value.alv_string);
			if (delta_date == (time_t)-1)
			{
				scp = sub_context_new();
				sub_var_set(scp, "Name", "%s", arglex_value.alv_string);
				fatal_intl(scp, i18n("date $name unknown"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_output:
			if (output)
				duplicate_option(copy_file_usage);
			switch (arglex())
			{
			default:
				option_needs_file(arglex_token_output, copy_file_usage);

			case arglex_token_stdio:
				output = "";
				break;

			case arglex_token_string:
				output = arglex_value.alv_string;
				break;
			}
			break;

		case arglex_token_branch:
			if (branch)
				duplicate_option(copy_file_usage);
			switch (arglex())
			{
			default:
				option_needs_number(arglex_token_branch, copy_file_usage);

			case arglex_token_number:
			case arglex_token_string:
				branch = arglex_value.alv_string;
				break;
			}
			break;

		case arglex_token_trunk:
			if (trunk)
				duplicate_option(copy_file_usage);
			++trunk;
			break;

		case arglex_token_read_only:
			if (read_only)
				duplicate_option(copy_file_usage);
			++read_only;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(copy_file_usage);
			break;

		case arglex_token_base_relative:
		case arglex_token_current_relative:
			user_relative_filename_preference_argument(copy_file_usage);
			break;
		}
		arglex();
	}
	if (!wl.nstrings)
	{
		error_intl(0, i18n("no file names"));
		copy_file_usage();
	}
	if (trunk)
	{
		if (branch)
		{
			mutually_exclusive_options
			(
				arglex_token_branch,
				arglex_token_trunk,
				copy_file_usage
			);
		}
		branch = "";
	}
	if ((delta_name || delta_number >= 0) && delta_date != (time_t)-1)
	{
		mutually_exclusive_options
		(
			arglex_token_delta,
			arglex_token_delta_date,
			copy_file_usage
		);
	}

	/*
	 * make sure output is unambiguous
	 */
	if (output)
	{
		if (wl.nstrings != 1)
		{
			scp = sub_context_new();
			sub_var_set(scp, "Number", "%ld", (long)wl.nstrings);
			sub_var_optional(scp, "Number");
			fatal_intl(scp, i18n("single file with -Output"));
			/* NOTREACHED */
			sub_context_delete(scp);
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
	 * locate which branch
	 */
	if (branch)
		pp2 = project_find_branch(pp, branch);
	else
		pp2 = pp;

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
	 *
	 * Also take a read lock on the baseline, to ensure that it does
	 * not change (aeip) for the duration of the build.
	 */
	if (!output)
	{
		change_cstate_lock_prepare(cp);
		project_baseline_read_lock_prepare(pp2);
		lock_take();

		log_open(change_logfile_get(cp), up, log_style);
	}
	cstate_data = change_cstate_get(cp);

	/*
	 * When there is no explicit output file:
	 * It is an error if the change is not in the being_developed state.
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
			wrong_state:
			change_fatal(cp, 0, i18n("bad cp state"));
		}
	}
	else
	{
		if (cstate_data->state != cstate_state_being_developed)
			goto wrong_state;
		if (change_is_a_branch(cp))
			change_fatal(cp, 0, i18n("bad branch cp"));
		if (!str_equal(change_developer_name(cp), user_name(up)))
			change_fatal(cp, 0, i18n("not developer"));
	}

	/*
	 * it is an error if the delta does not exist
	 */
	if (delta_name)
	{
		s1 = str_from_c(delta_name);
		delta_number = project_history_delta_by_name(pp2, s1, 0);
		str_free(s1);
	}
	if (delta_number >= 0)
	{
		delta_date = project_history_delta_to_timestamp(pp2, delta_number);
		if (delta_date == (time_t)-1)
		{
			scp = sub_context_new();
			sub_var_set(scp, "Name", "%ld", delta_number);
			change_fatal(cp, scp, i18n("no delta $name"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
	}

	/*
	 * build the list of places to look
	 * when resolving the file name
	 *
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
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
	 * 1. the absolute path of the file name is obtained
	 * 2. if the file is inside the search list
	 * 3. if neither, error
	 */
	config_seen = 0;
	config_name = str_from_c(THE_CONFIG_FILE);
	string_list_constructor(&wl2);
	number_of_errors = 0;
	for (j = 0; j < wl.nstrings; ++j)
	{
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
			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", wl.string[j]);
			change_error(cp, scp, i18n("$filename unrelated"));
			sub_context_delete(scp);
			++number_of_errors;
			continue;
		}
		project_file_dir(pp2, s2, &wl_in, &wl_out);
		if (delta_date != (time_t)-1)
			string_list_append_list(&wl_in, &wl_out);
		if (wl_in.nstrings)
		{
			int		used;

			/*
			 * if the user named a directory,
			 * add all of the source files in that directory,
			 * provided they are not already in the change.
			 */
			if (output)
			{
				scp = sub_context_new();
				sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_output));
				error_intl(scp, i18n("no dir with $name"));
				sub_context_delete(scp);
				++number_of_errors;
			}
			used = 0;
			for (k = 0; k < wl_in.nstrings; ++k)
			{
				string_ty	*s3;

				s3 = wl_in.string[k];
				if (stomp || !change_file_find(cp, s3))
				{
					if (string_list_member(&wl2, s3))
					{
						scp = sub_context_new();
						sub_var_set(scp, "File_Name", "%S", s3);
						change_error(cp, scp, i18n("too many $filename"));
						sub_context_delete(scp);
						++number_of_errors;
					}
					else
						string_list_append(&wl2, s3);
					if (str_equal(s3, config_name))
						++config_seen;
					++used;
				}
			}
			if (!used)
			{
				scp = sub_context_new();
				if (s2->str_length)
					sub_var_set(scp, "File_Name", "%S", s2);
				else
					sub_var_set(scp, "File_Name", ".");
				sub_var_set(scp, "Number", "%ld", (long)wl_in.nstrings);
				sub_var_optional(scp, "Number");
				change_error(cp, scp, i18n("directory $filename contains no relevant files"));
				sub_context_delete(scp);
				++number_of_errors;
			}
		}
		else
		{
			if (string_list_member(&wl2, s2))
			{
				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", s2);
				change_error(cp, scp, i18n("too many $filename"));
				sub_context_delete(scp);
				++number_of_errors;
			}
			else
				string_list_append(&wl2, s2);
			if (str_equal(s2, config_name))
				++config_seen;
		}
		string_list_destructor(&wl_in);
		string_list_destructor(&wl_out);
		str_free(s2);
	}
	string_list_destructor(&search_path);
	string_list_destructor(&wl);
	wl = wl2;
	str_free(config_name);

	/*
	 * ensure that each file
	 * 1. is not already part of the change
	 * 2. is in the baseline
	 */
	for (j = 0; j < wl.nstrings; ++j)
	{
		fstate_src	src_data;

		s1 = wl.string[j];
		if (change_file_find(cp, s1) && !stomp && !output)
		{
			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", s1);
			change_error(cp, scp, i18n("bad cp, file $filename dup"));
			sub_context_delete(scp);
			++number_of_errors;
			continue;
		}
		if (output)
		{
			fstate_src	c_src_data;

			/*
			 * OK to use a file that "almost" exists
			 * in combination with the -Output option
			 */
			c_src_data = change_file_find(cp, s1);
			if
			(
				c_src_data
			&&
				c_src_data->action == file_action_create
			)
				continue;
		}
		src_data = project_file_find(pp2, s1);
		if
		(
			!src_data
		||
			src_data->about_to_be_created_by
		||
			(delta_date == (time_t)-1 && src_data->deleted_by)
		)
		{
			scp = sub_context_new();
			src_data = project_file_find_fuzzy(pp2, s1);
			sub_var_set(scp, "File_Name", "%S", s1);
			if (src_data)
			{
				sub_var_set(scp, "Guess", "%S", src_data->file_name);
				project_error
				(
					pp2,
					scp,
					i18n("no $filename, closest is $guess")
				);
			}
			else
				project_error(pp2, scp, i18n("no $filename"));
			sub_context_delete(scp);
			++number_of_errors;
			continue;
		}
		if (src_data && src_data->usage == file_usage_build && !output)
		{
			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", s1);
			change_error(cp, scp, i18n("$filename is built"));
			sub_context_delete(scp);
			++number_of_errors;
		}
	}
	if (number_of_errors)
	{
		scp = sub_context_new();
		sub_var_set(scp, "Number", "%d", number_of_errors);
		sub_var_optional(scp, "Number");
		change_fatal(cp, scp, i18n("no files copied"));
		sub_context_delete(scp);
	}

	/*
	 * Determine the correct mode for the copied files.
	 */
	mode = (read_only ? 0444 : 0644) & ~change_umask(cp);

	/*
	 * Copy each file into the development directory.
	 * Create any necessary directories along the way.
	 *
	 * Add each file to the change file,
	 * or update the edit number.
	 */
	dd = change_development_directory_get(cp, 0);
	for (j = 0; j < wl.nstrings; ++j)
	{
		string_ty	*from;
		string_ty	*to;

		s1 = wl.string[j];
		delta_date_implies_edit_number = 0;
		if (delta_date != (time_t)-1)
		{
			/*
			 * find the edit number,
			 * given the file name and delta number
			 *
			 * NULL returned means that the file
			 * does not exist at the given delta.
			 */
			delta_date_implies_edit_number =
				project_delta_date_to_edit(pp2, delta_date, s1);
			if (delta_date_implies_edit_number)
			{
				fstate_src	psrc_data;

				/*
				 * Use the immediate project...
				 * the other branch may have been completed.
				 */
				psrc_data = project_file_find(pp, s1);
				if
				(
					psrc_data
				&&
					psrc_data->edit_number
				&&
					str_equal(psrc_data->edit_number, delta_date_implies_edit_number)
				&&
					psrc_data->action != file_action_remove
				)
				{
					/*
					 * do a simple file copy
					 * from the immediate project
					 */
					from = project_file_path(pp, s1);
					str_free(delta_date_implies_edit_number);
					delta_date_implies_edit_number = 0;
				}
				else
				{
					/*
					 * make a temporary file
					 */
					from = os_edit_filename(0);
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
						delta_date_implies_edit_number,
						from,
						up
					);
				}
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
			{
				os_mkdir_between(dd, s1, 02755);
				if (os_exists(to))
					os_unlink(to);
			}
			copy_whole_file(from, to, 0);

			/*
			 * set the file mode
			 */
			os_chmod(to, mode);

			/*
			 * clean up afterwards
			 */
			if (delta_date_implies_edit_number)
				os_unlink_errok(from);
			user_become_undo();
			str_free(from);
			str_free(to);
		}
		else
		{
			if (cstate_data->state == cstate_state_being_integrated)
				from = str_format("%S/%S", change_integration_directory_get(cp, 0), s1);
			else
				from = project_file_path(pp2, s1);
			if (output)
				to = str_from_c(output);
			else
				to = str_format("%S/%S", dd, s1);

			/*
			 * copy the file
			 */
			user_become(up);
			if (!output)
			{
				os_mkdir_between(dd, s1, 02755);
				if (os_exists(to))
					os_unlink(to);
			}
			copy_whole_file(from, to, 0);

			/*
			 * set the file mode
			 */
			os_chmod(to, mode);
			user_become_undo();

			/*
			 * clean up afterwards
			 */
			str_free(from);
			str_free(to);
		}

		if (!output)
		{
			fstate_src	c_src_data;
			fstate_src	p_src_data;

			p_src_data = project_file_find(pp2, s1);
			assert(p_src_data);
			assert(p_src_data->edit_number);
			assert(p_src_data->edit_number_origin);
			c_src_data = change_file_find(cp, s1);
			if (!c_src_data)
			{
				c_src_data = change_file_new(cp, s1);
				c_src_data->action =
					(
						read_only
					?
						file_action_insulate
					:
						file_action_modify
					);
				c_src_data->usage = p_src_data->usage;

				/*
				 * The change now has at least one test,
				 * so cancel any testing exemption.
				 * (But test_baseline_exempt is still viable.)
				 */
				if (!read_only)
				{
					switch (c_src_data->usage)
					{
					case file_usage_test:
					case file_usage_manual_test:
						change_rescind_test_exemption(cp);
						break;

					case file_usage_source:
					case file_usage_build:
						break;
					}
				}
			}

			/*
			 * p_src_data->edit_number
			 *	The head revision of the branch.
			 * p_src_data->edit_number_origin
			 *	The version originally copied.
			 *
			 * c_src_data->edit_number
			 *	Not meaningful until after integrate pass.
			 * c_src_data->edit_number_origin
			 *	The version originally copied.
			 * c_src_data->edit_number_origin_new
			 *	Updates branch edit_number_origin on
			 *	integrate pass.
			 */
			if (c_src_data->edit_number)
			{
				str_free(c_src_data->edit_number);
				c_src_data->edit_number = 0;
			}
			if (c_src_data->edit_number_origin)
			{
				str_free(c_src_data->edit_number_origin);
				c_src_data->edit_number_origin = 0;
			}
			if (c_src_data->edit_number_origin_new)
			{
				str_free(c_src_data->edit_number_origin_new);
				c_src_data->edit_number_origin_new = 0;
			}
			assert(p_src_data->edit_number);
			assert(p_src_data->edit_number_origin);
			if (delta_date_implies_edit_number)
			{
				c_src_data->edit_number_origin =
				     str_copy(delta_date_implies_edit_number);
			}
			else
			{
				c_src_data->edit_number_origin =
					str_copy(p_src_data->edit_number);
			}

			/*
			 * Copying the config file into a change
			 * invalidates all of the file fingerprints.
			 * This is because the diff command,
			 * test_command, build_command, etc, could be
			 * changed when the config file is edited.
			 */
			if (config_seen && c_src_data->file_fp)
			{
				fingerprint_type.free(c_src_data->file_fp);
				c_src_data->file_fp = 0;
			}
		}
		if (delta_date_implies_edit_number)
			str_free(delta_date_implies_edit_number);

		/*
		 * verbose progress message
		 */
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", s1);
		change_verbose(cp, scp, i18n("copied $filename"));
		sub_context_delete(scp);
	}

	if (!output)
	{
		/*
		 * the number of files changed,
		 * so stomp on the validation fields.
		 */
		change_build_times_clear(cp);

		/*
		 * update the copyright years
		 */
		change_copyright_years_now(cp);
	}

	/*
	 * release the locks
	 */
	if (!output)
	{
		/*
		 * run the change file command
		 * and the project file command if necessary
		 */
		change_run_change_file_command(cp, &wl, up);
		change_run_project_file_command(cp, up);

		change_cstate_write(cp);
		commit();
		lock_release();
	}

	/*
	 * verbose success message
	 */
	scp = sub_context_new();
	sub_var_set(scp, "Number", "%ld", (long)wl.nstrings);
	sub_var_optional(scp, "Number");
	change_verbose(cp, scp, i18n("copy file complete"));
	sub_context_delete(scp);

	/*
	 * run the change file command
	 */
	string_list_destructor(&wl);
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

	case arglex_token_independent:
		copy_file_independent();
		break;
	}
	trace((/*{*/"}\n"));
}
