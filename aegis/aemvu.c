/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate aemvus
 */

#include <ac/stdio.h>

#include <aemvu.h>
#include <ael/change/files.h>
#include <arglex2.h>
#include <change.h>
#include <change/file.h>
#include <change/branch.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <os.h>
#include <pconf.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <zero.h>


static void move_file_undo_usage _((void));

static void
move_file_undo_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf
	(
		stderr,
		"usage: %s -MoVe_file_Undo [ <option>... ] <filename>...\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -MoVe_file_Undo -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -MoVe_file_Undo -Help\n", progname);
	quit(1);
}


static void move_file_undo_help _((void));

static void
move_file_undo_help()
{
	help("aemvu", move_file_undo_usage);
}


static void move_file_undo_list _((void));

static void
move_file_undo_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("move_file_undo_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(move_file_undo_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, move_file_undo_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, move_file_undo_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set_long(scp, "Number", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, move_file_undo_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, move_file_undo_usage);
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


static void move_file_undo_main _((void));

static void
move_file_undo_main()
{
	sub_context_ty	*scp;
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
	int		config_seen;
	string_ty	*config_name;
	int		number_of_errors;
	string_list_ty	search_path;
	int		mend_symlinks;
	pconf		pconf_data;
	int		based;
	string_ty	*base;
	string_list_ty	wl_nfu;
	string_list_ty	wl_ntu;
	string_list_ty	wl_rmu;

	trace(("move_file_undo_main()\n{\n"/*}*/));
	string_list_constructor(&wl);
	project_name = 0;
	change_number = 0;
	log_style = log_style_append_default;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(move_file_undo_usage);
			continue;

		case arglex_token_directory:
			if (arglex() != arglex_token_string)
				option_needs_dir(arglex_token_directory, move_file_undo_usage);
			goto get_file_names;


		case arglex_token_file:
			if (arglex() != arglex_token_string)
				option_needs_files(arglex_token_file, move_file_undo_usage);
			/* fall through... */

		case arglex_token_string:
			get_file_names:
			s2 = str_from_c(arglex_value.alv_string);
			string_list_append(&wl, s2);
			str_free(s2);
			break;

		case arglex_token_keep:
		case arglex_token_interactive:
		case arglex_token_no_keep:
			user_delete_file_argument(move_file_undo_usage);
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, move_file_undo_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, move_file_undo_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				scp = sub_context_new();
				sub_var_set_long(scp, "Number", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, move_file_undo_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, move_file_undo_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (log_style == log_style_none)
				duplicate_option(move_file_undo_usage);
			log_style = log_style_none;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(move_file_undo_usage);
			break;

		case arglex_token_base_relative:
		case arglex_token_current_relative:
			user_relative_filename_preference_argument(move_file_undo_usage);
			break;
		}
		arglex();
	}
	if (!wl.nstrings)
		fatal_intl(0, i18n("no file names"));

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
		change_fatal(cp, 0, i18n("bad cp undo state"));
	if (change_is_a_branch(cp))
		change_fatal(cp, 0, i18n("bad cp undo branch"));
	if (!str_equal(change_developer_name(cp), user_name(up)))
		change_fatal(cp, 0, i18n("not developer"));

	/*
	 * resolve the path of each file
	 * 1.	the absolute path of the file name is obtained
	 * 2.	if the file is inside the development directory, ok
	 * 3.	if the file is inside the baseline, ok
	 * 4.	if neither, error
	 */
	config_seen = 0;
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

	config_name = str_from_c(THE_CONFIG_FILE);
	string_list_constructor(&wl2);
	number_of_errors = 0;
	for (j = 0; j < wl.nstrings; ++j)
	{
		string_list_ty		wl_in;

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
			sub_var_set_string(scp, "File_Name", wl.string[j]);
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
			 * source files in this change in that
			 * directory, provided they were added using
			 * the aemv command.
			 */
			used = 0;
			for (k = 0; k < wl_in.nstrings; ++k)
			{
				string_ty	*s3;
				fstate_src	src_data;

				s3 = wl_in.string[k];
				src_data = change_file_find(cp, s3);
				assert(src_data);
				if (src_data && src_data->move)
				{
					if (string_list_member(&wl2, s3))
					{
						scp = sub_context_new();
						sub_var_set_string(scp, "File_Name", s3);
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
					sub_var_set_string(scp, "File_Name", s2);
				else
					sub_var_set_charstar(scp, "File_Name", ".");
				sub_var_set_long(scp, "Number", (long)wl_in.nstrings);
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
				scp = sub_context_new();
				sub_var_set_string(scp, "File_Name", s2);
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
		str_free(s2);
	}
	string_list_destructor(&wl);
	wl = wl2;
	str_free(config_name);
	string_list_destructor(&search_path);

	/*
	 * ensure that each file
	 * 1. is already part of the change
	 * 2. is being moved by this change
	 */
	for (j = 0; j < wl.nstrings; ++j)
	{
		fstate_src	src_data;

		s1 = wl.string[j];
		src_data = change_file_find(cp, s1);
		if (!src_data)
		{
			scp = sub_context_new();
			src_data = change_file_find_fuzzy(cp, s1);
			sub_var_set_string(scp, "File_Name", s1);
			if (src_data)
			{
				sub_var_set_string(scp, "Guess", src_data->file_name);
				change_error
				(
					cp,
					scp,
					i18n("no $filename, closest is $guess")
				);
			}
			else
				change_error(cp, scp, i18n("no $filename"));
			sub_context_delete(scp);
			++number_of_errors;
			continue;
		}
		if (src_data->move)
		{
			/*
			 * Add the other half of the move,
			 * if it isn't there already.
			 */
			string_list_append_unique(&wl, src_data->move);
		}
		else
		{
			/*
			 * If there is no "other half", then it
			 * wasn't added to the change as a file move,
			 * so complain.
			 */
			scp = sub_context_new();
			sub_var_set_string(scp, "File_Name", s1);
			change_error(cp, scp, i18n("bad mv undo $filename"));
			sub_context_delete(scp);
			++number_of_errors;
			continue;
		}
		if (config_seen && src_data->file_fp)
		{
			fingerprint_type.free(src_data->file_fp);
			src_data->file_fp = 0;
		}
	}
	if (number_of_errors)
	{
		scp = sub_context_new();
		sub_var_set_long(scp, "Number", number_of_errors);
		sub_var_optional(scp, "Number");
		change_fatal(cp, scp, i18n("no files uncopied"));
		sub_context_delete(scp);
	}

	/*
	 * Figure out if we need to mend symlinks as we go.
	 */
	pconf_data = change_pconf_get(cp, 0);
	mend_symlinks =
		(
			pconf_data->create_symlinks_before_build
		&&
			!pconf_data->remove_symlinks_after_build
		);

	/*
	 * Remove each file from the development directory,
	 * if it still exists.
	 * Remove the difference file, too.
	 */
	for (j = 0; j < wl.nstrings; ++j)
	{
		int		exists;

		s1 = wl.string[j];
		s2 = change_file_path(cp, s1);
		assert(s2);
		user_become(up);
		exists = os_exists(s2);
		user_become_undo();

		/*
		 * delete the file if it exists
		 * and the user wants us to
		 */
		if (exists && user_delete_file_query(up, s1, 0))
		{
			fstate_src	psrc_data;

			if (mend_symlinks)
			{
				psrc_data = project_file_find(pp, s1);
				if
				(
					psrc_data
				&&
					(
						psrc_data->deleted_by
					||
					       psrc_data->about_to_be_created_by
					)
				)
					psrc_data = 0;
			}
			else
				psrc_data = 0;

			if (psrc_data)
			{
				string_ty *blf;

				/*
				 * This is not as robust in the face of
				 * errors as using commit.  Its merit
				 * is its simplicity.
				 *
				 * Also, the rename-and-delete shenanigans
				 * take a long time over NFS, and users
				 * expect this to be fast.
				 */
				blf = project_file_path(pp, s1);
				assert(blf);
				user_become(up);
				os_unlink(s2);
				os_symlink(blf, s2);
				user_become_undo();
				str_free(blf);
			}
			else
			{
				user_become(up);
				commit_unlink_errok(s2);
				user_become_undo();
			}
		}

		/*
		 * always delete the difference file
		 * and the merge backup file
		 */
		user_become(up);
		s1 = str_format("%S,D", s2);
		if (os_exists(s1))
			commit_unlink_errok(s1);
		str_free(s1);

		/*
		 * always delete the backup merge file
		 */
		s1 = str_format("%S,B", s2);
		if (os_exists(s1))
			commit_unlink_errok(s1);
		str_free(s1);
		user_become_undo();
		str_free(s2);
	}

	/*
	 * Remove each file from the change file,
	 * and write it back out.
	 */
	string_list_constructor(&wl_nfu);
	string_list_constructor(&wl_ntu);
	string_list_constructor(&wl_rmu);
	for (j = 0; j < wl.nstrings; ++j)
	{
		string_ty	*s;
		fstate_src	c_src;

		s = wl.string[j];
		c_src = change_file_find(cp, s);
		assert(c_src);
		if (c_src && c_src->action == file_action_create)
		{
			switch (c_src->usage)
			{
			case file_usage_test:
			case file_usage_manual_test:
				string_list_append(&wl_ntu, s);
				break;

			case file_usage_source:
			case file_usage_build:
				string_list_append(&wl_nfu, s);
				break;
			}
		}
		else
			string_list_append(&wl_rmu, s);
		change_file_remove(cp, s);
	}

	/*
	 * the number of files changed,
	 * so stomp on the validation fields.
	 */
	change_build_times_clear(cp);

	/*
	 * run the change file command
	 * and the project file command if necessary
	 */
	if (wl_nfu.nstrings)
		change_run_new_file_undo_command(cp, &wl_nfu, up);
	if (wl_ntu.nstrings)
		change_run_new_file_undo_command(cp, &wl_ntu, up);
	if (wl_rmu.nstrings)
		change_run_remove_file_undo_command(cp, &wl_rmu, up);
	change_run_project_file_command(cp, up);
	string_list_destructor(&wl_nfu);
	string_list_destructor(&wl_ntu);
	string_list_destructor(&wl_rmu);

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
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", wl.string[j]);
		change_verbose(cp, scp, i18n("$filename gone"));
		sub_context_delete(scp);
	}

	string_list_destructor(&wl);
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
move_file_undo()
{
	trace(("move_file_undo()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		move_file_undo_main();
		break;

	case arglex_token_help:
		move_file_undo_help();
		break;

	case arglex_token_list:
		move_file_undo_list();
		break;
	}
	trace((/*{*/"}\n"));
}
