/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995 Peter Miller;
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
 * MANIFEST: functions to impliment copy file undo
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>

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
	fprintf
	(
		stderr,
		"usage: %s -CoPy_file_Undo [ <option>... ] <filename>...\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -CoPy_file_Undo -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -CoPy_file_Undo -Help\n", progname);
	quit(1);
}


static void copy_file_undo_help _((void));

static void
copy_file_undo_help()
{
	static char *text[] =
	{
#include <../man1/aecpu.h>
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
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;
	int		config_seen;
	string_ty	*config_name;
	string_ty	*dd;
	string_ty	*bl;
	int		unchanged;
	int		number_of_errors;

	trace(("copy_file_undo_main()\n{\n"/*}*/));
	wl_zero(&wl);
	project_name = 0;
	change_number = 0;
	nolog = 0;
	unchanged = 0;
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
			{
				fatal
				(
					"file \"%s\" named more than once",
					arglex_value.alv_string
				);
			}
			wl_append(&wl, s2);
			str_free(s2);
			break;

		case arglex_token_keep:
		case arglex_token_interactive:
		case arglex_token_no_keep:
			user_delete_file_argument();
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

		case arglex_token_unchanged:
			if (unchanged)
				goto duplicate;
			unchanged = 1;
			break;
		}
		arglex();
	}
	if (!unchanged && !wl.wl_nwords)
		fatal("no filenames specified");

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

	/*
	 * It is an error if the change is not in the in_development state.
	 * It is an error if the change is not assigned to the current user.
	 */
	if (cstate_data->state != cstate_state_being_developed)
	{
		change_fatal
		(
			cp,
"this change is in the '%s' state, it must be in the \
'being developed' state to be able to remove a copied file from it",
			cstate_state_ename(cstate_data->state)
		);
	}
	if (!str_equal(change_developer_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
   "user \"%S\" is not the developer, only user \"%S\" may work on this change",
			user_name(up),
			change_developer_name(cp)
		);
	}

	/*
	 * If no files were named and the -unchanged option was used,
	 * add all of the modified files in the change.
	 * It is an error if there are none.
	 */
	dd = change_development_directory_get(cp, 1);
	if (!wl.wl_nwords)
	{
		assert(unchanged);
		for (j = 0; j < cstate_data->src->length; ++j)
		{
			cstate_src	src;

			src = cstate_data->src->list[j];
			if (src->action != file_action_modify)
				continue;
			s1 = str_format("%S/%S", dd, src->file_name);
			wl_append(&wl, s1);
			str_free(s1);
		}
		if (!wl.wl_nwords)
			change_fatal(cp, "no files being modified");
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
		if (str_equal(s2, config_name))
			config_seen++;
	}
	str_free(config_name);

	/*
	 * ensure that each file
	 * 1. is already part of the change
	 * 2. is being modified by this change
	 */
	number_of_errors = 0;
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		cstate_src	src_data;

		s1 = wl.wl_word[j];
		src_data = change_src_find(cp, s1);
		if (!src_data)
		{
			src_data = change_src_find_fuzzy(cp, s1);
			if (src_data)
			{
				change_error
				(
					cp,
		       "file \"%S\" not in change, closest was the \"%S\" file",
					s1,
					src_data->file_name
				);
			}
			else
			{
				change_error
				(
					cp,
					"file \"%S\" not in change",
					s1
				);
			}
			++number_of_errors;
			continue;
		}
		if (src_data->action != file_action_modify)
		{
			change_error
			(
				cp,
		    "file \"%S\" was not added to this change using -CoPy_file",
				s1
			);
			++number_of_errors;
			continue;
		}
		if (config_seen)
			src_data->diff_time = 0;
	}
	if (number_of_errors)
	{
		change_fatal
		(
			cp,
			"found %d fatal error%s, no copied files removed",
			number_of_errors,
			(number_of_errors == 1 ? "" : "s")
		);
	}

	/*
	 * Remove each file from the development directory,
	 * if it still exists.
	 * Remove the difference file, too.
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		int		exists;

		s1 = wl.wl_word[j];
		s2 = str_format("%S/%S", dd, s1);
		user_become(up);
		exists = os_exists(s2);
		user_become_undo();

		/*
		 * skip the changed files
		 * if the user asked us to work on unchanged files
		 */
		if (unchanged && exists)
		{
			string_ty	*blf;
			int		different;

			blf = str_format("%S/%S", bl, s1);
			user_become(up);
			different = files_are_different(s2, blf);
			user_become_undo();
			str_free(blf);
			if (different)
			{
				wl_delete(&wl, s1);
				--j;
				str_free(s2);
				continue;
			}
		}

		/*
		 * delete the file if it exists
		 * and the users wants us to
		 */
		if (exists && user_delete_file_query(up, s1, 0))
		{
			user_become(up);
			commit_unlink_errok(s2);
			user_become_undo();
		}
		str_free(s2);

		/*
		 * always delete the difference file
		 */
		s2 = str_format("%S/%S,D", dd, s1);
		user_become(up);
		if (os_exists(s2))
			commit_unlink_errok(s2);
		user_become_undo();
		str_free(s2);
	}

	/*
	 * Remove each file from the change file,
	 * and write it back out.
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
		change_src_remove(cp, wl.wl_word[j]);

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
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		change_verbose
		(
			cp,
			"file \"%S\" no longer in change",
			wl.wl_word[j]
		);
	}

	/*
	 * run the change file command
	 */
	if (!nolog)
		log_open(change_logfile_get(cp), up, log_style_append);
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
