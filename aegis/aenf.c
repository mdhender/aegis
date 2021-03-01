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
 * MANIFEST: functions to implement new file
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <ac/fcntl.h>
#include <ac/unistd.h>

#include <ael.h>
#include <aenf.h>
#include <arglex2.h>
#include <change_bran.h>
#include <change_file.h>
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
#include <project_file.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <str_list.h>


static void new_file_usage _((void));

static void
new_file_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf
	(
		stderr,
		"usage: %s -New_File <filename>... [ <option>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -New_File -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -New_File -Help\n", progname);
	quit(1);
}


static void new_file_help _((void));

static void
new_file_help()
{
	help("aenf", new_file_usage);
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
				option_needs_number(arglex_token_change, new_file_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, new_file_usage);
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
				option_needs_name(arglex_token_project, new_file_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, new_file_usage);
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
	string_ty	*dd;
	string_list_ty	wl;
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
	int		generated;
	int		nerrs;
	string_list_ty	search_path;
	string_ty	*config_name;
	string_list_ty	wl2;
	int		based;
	string_ty	*base;

	trace(("new_file_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	generated = 0;
	string_list_constructor(&wl);
	log_style = log_style_append_default;
	nerrs = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_file_usage);
			continue;

		case arglex_token_file:
			if (arglex() != arglex_token_string)
				new_file_usage();
			/* fall through... */

		case arglex_token_string:
			s2 = str_from_c(arglex_value.alv_string);
			string_list_append(&wl, s2);
			str_free(s2);
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, new_file_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, new_file_usage);
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
				option_needs_name(arglex_token_project, new_file_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, new_file_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (log_style == log_style_none)
				duplicate_option(new_file_usage);
			log_style = log_style_none;
			break;

		case arglex_token_build:
			if (generated)
				duplicate_option(new_file_usage);
			generated = 1;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(new_file_usage);
			break;

		case arglex_token_base_relative:
		case arglex_token_current_relative:
			user_relative_filename_preference_argument(new_file_usage);
			break;
		}
		arglex();
	}
	if (nerrs)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Number", "%d", nerrs);
		sub_var_optional(scp, "Number");
		fatal_intl(scp, i18n("no new files"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!wl.nstrings)
	{
		error_intl(0, i18n("no file names"));
		new_file_usage();
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
	 * It is an error if the change is not in the being_developed state.
	 * It is an error if the change is not assigned to the current user.
	 */
	if (cstate_data->state != cstate_state_being_developed)
		change_fatal(cp, 0, i18n("bad nf state"));
	if (change_is_a_branch(cp))
		change_fatal(cp, 0, i18n("bad nf branch"));
	if (!str_equal(change_developer_name(cp), user_name(up)))
		change_fatal(cp, 0, i18n("not developer"));

	/*
	 * Search list for resolving filenames.
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
	string_list_constructor(&wl2);
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
			change_error(cp, scp, i18n("$filename unrelated"));
			sub_context_delete(scp);
			++nerrs;
			continue;
		}
		if (string_list_member(&wl2, s2))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", s2);
			change_error(cp, scp, i18n("too many $filename"));
			sub_context_delete(scp);
			++nerrs;
		}
		else
			string_list_append(&wl2, s2);
		str_free(s2);
	}
	string_list_destructor(&search_path);
	string_list_destructor(&wl);
	wl = wl2;

	/*
	 * ensure that each file
	 * 1. is not already part of the change
	 * 2. is not already part of the baseline
	 */
	for (j = 0; j < wl.nstrings; ++j)
	{
		fstate_src	src_data;

		s1 = wl.string[j];
		if (change_file_find(cp, s1))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", s1);
			change_error(cp, scp, i18n("file $filename dup"));
			sub_context_delete(scp);
			++nerrs;
		}
		else
		{
			src_data = project_file_find(pp, s1);
			if
			(
				src_data
			&&
				!src_data->deleted_by
			&&
				!src_data->about_to_be_created_by
			)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", s1);
				project_error
				(
					pp,
					scp,
					i18n("$filename in baseline")
				);
				sub_context_delete(scp);
				++nerrs;
			}
		}
	}

	/*
	 * check that each filename is OK
	 */
	config_name = str_from_c(THE_CONFIG_FILE);
	for (j = 0; j < wl.nstrings; ++j)
	{
		string_ty	*e;

		if (generated && str_equal(wl.string[j], config_name))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", wl.string[j]);
			change_error(cp, scp, i18n("may not build $filename"));
			sub_context_delete(scp);
			++nerrs;
		}
		e = change_filename_check(cp, wl.string[j], 1);
		if (e)
		{
			sub_context_ty	*scp;

			/*
			 * no internationalization if the error string
			 * required, this is done inside the
			 * change_filename_check function.
			 */
			scp = sub_context_new();
			sub_var_set(scp, "Message", "%S", e);
			change_error(cp, scp, i18n("$message"));
			sub_context_delete(scp);
			++nerrs;
			str_free(e);
		}
	}
	str_free(config_name);
	if (nerrs)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Number", "%d", nerrs);
		sub_var_optional(scp, "Number");
		change_fatal(cp, scp, i18n("no new files"));
		sub_context_delete(scp);
	}

	/*
	 * Create each file in the development directory,
	 * if it does not already exist.
	 * Create any necessary directories along the way.
	 */
	dd = change_development_directory_get(cp, 0);
	user_become(up);
	for (j = 0; j < wl.nstrings; ++j)
	{
		s1 = wl.string[j];
		trace(("does %s exist?\n", s1->str_text));
		os_mkdir_between(dd, s1, 02755);
		s2 = str_format("%S/%S", dd, s1);
		if (os_symlink_query(s2))
			os_unlink(s2);
		if (!os_exists(s2))
		{
			int		fd;
			string_ty	*template;

			trace(("create %s\n", s2->str_text));
			user_become_undo();
			template = change_file_template(cp, s1);
			user_become(up);
			fd = glue_creat(s2->str_text, 0666);
			if (fd < 0)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_errno_set(scp);
				sub_var_set(scp, "File_Name", "%S", s2);
				fatal_intl(scp, i18n("create $filename: $errno"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			if (template)
			{
				glue_write
				(
					fd,
					template->str_text,
					template->str_length
				);
				if
				(
					template->str_length
				&&
					(
						template->str_text
						[
							template->str_length - 1
						]
					!=
						'\n'
					)
				)
					glue_write(fd, "\n", 1);
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
	for (j = 0; j < wl.nstrings; ++j)
	{
		fstate_src	src_data;

		src_data = change_file_new(cp, wl.string[j]);
		src_data->action = file_action_create;
		if (generated)
			src_data->usage = file_usage_build;
		else
			src_data->usage = file_usage_source;
	}

	/*
	 * the number of files changed, or the version did,
	 * so stomp on the validation fields.
	 */
	change_build_times_clear(cp);

	/*
	 * update the copyright years
	 */
	change_copyright_years_now(cp);

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
		change_verbose(cp, scp, i18n("new file $filename completed"));
		sub_context_delete(scp);
	}
	string_list_destructor(&wl);
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
