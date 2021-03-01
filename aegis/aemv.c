/*
 *	aegis - project change supervisor
 *	Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998 Peter Miller;
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
 * MANIFEST: functions to implement move file
 */

#include <stdio.h>

#include <ael.h>
#include <aemv.h>
#include <arglex2.h>
#include <change_bran.h>
#include <change_file.h>
#include <commit.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project_file.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <str_list.h>


static void move_file_usage _((void));

static void
move_file_usage()
{
	char		*progname;

	progname = progname_get();
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
	help("aemv", move_file_usage);
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
				option_needs_number(arglex_token_change, move_file_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, move_file_usage);
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
				option_needs_name(arglex_token_project, move_file_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, move_file_usage);
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


static void move_file_innards _((user_ty *, change_ty *, string_ty *,
	string_ty *));

static void
move_file_innards(up, cp, old_name, new_name)
	user_ty		*up;
	change_ty	*cp;
	string_ty	*old_name;
	string_ty	*new_name;
{
	project_ty	*pp;
	fstate_src	p_src_data;
	fstate_src	pn_src_data;
	fstate_src	c_src_data;
	static string_ty *config_name;
	string_ty	*from;
	string_ty	*to;
	string_ty	*dd;
	int		mode;

	pp = cp->pp;
	if (!config_name)
		config_name = str_from_c(THE_CONFIG_FILE);

	/*
	 * the old file may not be in the change already
	 */
	if (change_file_find(cp, old_name))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", old_name);
		change_fatal(cp, scp, i18n("file $filename dup"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * the old file must be in the baseline
	 */
	p_src_data = project_file_find(pp, old_name);
	if
	(
		!p_src_data
	||
		p_src_data->about_to_be_created_by
	||
		p_src_data->deleted_by
	)
	{
		p_src_data = project_file_find_fuzzy(pp, old_name);
		if (p_src_data)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", old_name);
			sub_var_set(scp, "Guess", "%S", p_src_data->file_name);
			project_fatal(pp, scp, i18n("no $filename, closest is $guess"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
		else
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", old_name);
			project_fatal(pp, scp, i18n("no $filename"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
	}

	/*
	 * you may not move the config file
	 */
	if (str_equal(old_name, config_name))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", config_name);
		project_fatal(pp, scp, i18n("no move $filename"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * You may not move a file on top of itself (use aecp!).
	 */
	if (str_equal(old_name, new_name))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", old_name);
		change_fatal(cp, scp, i18n("nil move $filename"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * the new file must not already be part of the change
	 */
	if (change_file_find(cp, new_name))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", new_name);
		change_fatal(cp, scp, i18n("file $filename dup"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * the new file must not be part of the baseline
	 */
	pn_src_data = project_file_find(pp, new_name);
	if
	(
		pn_src_data
	&&
		!pn_src_data->about_to_be_created_by
	&&
		!pn_src_data->deleted_by
	)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", new_name);
		project_fatal(pp, scp, i18n("$filename in baseline"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * Add the files to the change
	 */
	assert(p_src_data);
	c_src_data = change_file_new(cp, old_name);
	c_src_data->action = file_action_remove;
	c_src_data->usage = p_src_data->usage;
	c_src_data->move = str_copy(new_name);
	assert(p_src_data->edit_number);
	if (p_src_data->edit_number)
		c_src_data->edit_number_origin =
			str_copy(p_src_data->edit_number);

	c_src_data = change_file_new(cp, new_name);
	c_src_data->action = file_action_create;
	c_src_data->usage = p_src_data->usage;
	c_src_data->move = str_copy(old_name);

	/*
	 * If the file is built, we are done.
	 */
	if (c_src_data->usage == file_usage_build)
		return;

	/*
	 * Copy the file into the development directory.
	 * Create any necessary directories along the way.
	 */
	from = project_file_path(pp, old_name);
	assert(from);
	to = change_file_path(cp, new_name);
	assert(to);
	dd = change_development_directory_get(cp, 0);
	mode = 0644 & ~change_umask(cp);

	/*
	 * Copy the file.
	 *
	 * Note: the file copy destroys anything in the development
	 * direcory at both ``from'' and ``to''.
	 */
	user_become(up);
	os_mkdir_between(dd, new_name, 02755);
	undo_unlink_errok(to);
	if (os_exists(to))
		os_unlink(to);
	copy_whole_file(from, to, 0);
	str_free(from);
	os_mkdir_between(dd, old_name, 02755);
	user_become_undo();
	from = change_file_path(cp, old_name);
	user_become(up);
	undo_unlink_errok(from);
	if (os_exists(from))
		os_unlink(from);
	os_junkfile(from, mode);
	user_become_undo();
	str_free(from);
	str_free(to);
}


static void move_file_main _((void));

static void
move_file_main()
{
	sub_context_ty	*scp;
	string_ty	*old_name;
	string_ty	*new_name;
	string_ty	*s1;
	string_ty	*s2;
	cstate		cstate_data;
	string_list_ty	wl;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	log_style_ty	log_style;
	user_ty		*up;
	size_t		k;
	string_list_ty	search_path;
	string_list_ty	wl_in;

	trace(("move_file_main()\n{\n"/*}*/));
	old_name = 0;
	new_name = 0;
	project_name = 0;
	change_number = 0;
	log_style = log_style_append_default;
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
				fatal_intl(0, i18n("too many files"));
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, move_file_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, move_file_usage);
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
			if (project_name)
				duplicate_option(move_file_usage);
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, move_file_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (log_style == log_style_none)
				duplicate_option(move_file_usage);
			log_style = log_style_none;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(move_file_usage);
			break;
		}
		arglex();
	}
	if (!old_name || !new_name)
	{
		error_intl(0, i18n("too few files named"));
		move_file_usage();
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

	/*
	 * It is an error if the change is not in the in_development state.
	 * It is an error if the change is not assigned to the current user.
	 */
	if (cstate_data->state != cstate_state_being_developed)
		change_fatal(cp, 0, i18n("bad mv state"));
	if (change_is_a_branch(cp))
		change_fatal(cp, 0, i18n("bad branch cp"));
	if (!str_equal(change_developer_name(cp), user_name(up)))
		change_fatal(cp, 0, i18n("not developer"));

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

	change_search_path_get(cp, &search_path, 1);
	assert(old_name->str_text[0] == '/');
	assert(new_name->str_text[0] == '/');

	s2 = 0;
	for (k = 0; k < search_path.nstrings; ++k)
	{
		s2 = os_below_dir(search_path.string[k], old_name);
		if (s2)
			break;
	}
	if (!s2)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", old_name);
		change_fatal(cp, scp, i18n("$filename unrelated"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	str_free(old_name);
	old_name = s2;

	s2 = 0;
	for (k = 0; k < search_path.nstrings; ++k)
	{
		s2 = os_below_dir(search_path.string[k], new_name);
		if (s2)
			break;
	}
	if (!s2)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", new_name);
		change_fatal(cp, scp, i18n("$filename unrelated"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	str_free(new_name);
	new_name = s2;
	string_list_destructor(&search_path);

	/*
	 * If the old file was a directory, then move all of its
	 * contents into the new directory.  If the old file was not a
	 * directory, just move it.  All checks to see if the action is
	 * valid are done in the inner function.
	 */
	project_file_dir(pp, old_name, &wl_in, 0);
	if (wl_in.nstrings)
	{
		size_t		j;

		for (j = 0; j < wl_in.nstrings; ++j)
		{
			string_ty	*filename_old;
			string_ty	*filename_tail;
			string_ty	*filename_new;

			/*
			 * Note: old_name and new_name will be empty
			 * strings if they refer to the top of the
			 * development directory tree.
			 */
			filename_old = wl_in.string[j];
			if (old_name->str_length)
			{
				filename_tail = os_below_dir(old_name, filename_old);
				if (!filename_tail)
					this_is_a_bug();
			}
			else
			{
				/* top-level directory */
				filename_tail = str_copy(filename_old);
			}
			if (new_name->str_length)
				filename_new = str_format("%S/%S", new_name, filename_tail);
			else
				filename_new = str_copy(filename_tail);
			str_free(filename_tail);

			/*
			 * move the file
			 */
			move_file_innards(up, cp, filename_old, filename_new);
			str_free(filename_old);
			str_free(filename_new);
		}
	}
	else
		move_file_innards(up, cp, old_name, new_name);
	string_list_destructor(&wl_in);

	/*
	 * the number of files changed,
	 * so stomp on the validation fields.
	 */
	change_build_times_clear(cp);

	/*
	 * release the locks
	 */
	change_cstate_write(cp);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	scp = sub_context_new();
	sub_var_set(scp, "File_Name1", "%S", old_name);
	sub_var_set(scp, "File_Name2", "%S", new_name);
	change_verbose(cp, scp, i18n("move $filename1 to $filename2 complete"));
	sub_context_delete(scp);

	/*
	 * run the change file command
	 */
	log_open(change_logfile_get(cp), up, log_style);
	string_list_constructor(&wl);
	string_list_append(&wl, old_name);
	string_list_append(&wl, new_name);
	change_run_change_file_command(cp, &wl, up);
	string_list_destructor(&wl);
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
