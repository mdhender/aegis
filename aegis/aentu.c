/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994 Peter Miller.
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
 * MANIFEST: functions to implement new test undo
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>

#include <ael.h>
#include <aentu.h>
#include <arglex2.h>
#include <change.h>
#include <col.h>
#include <commit.h>
#include <error.h>
#include <glue.h>
#include <help.h>
#include <lock.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <user.h>
#include <word.h>


static void new_test_undo_usage _((void));

static void
new_test_undo_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -New_Test_Undo <filename>... [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Test_Undo -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Test_Undo -Help\n", progname);
	quit(1);
}


static void new_test_undo_help _((void));

static void
new_test_undo_help()
{
	static char *text[] =
	{
#include <../man1/aentu.h>
	};

	help(text, SIZEOF(text), new_test_undo_usage);
}


static void new_test_undo_list _((void));

static void
new_test_undo_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("new_test_undo_list()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_test_undo_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				new_test_undo_usage();
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
				new_test_undo_usage();
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


static void new_test_undo_main _((void));

static void
new_test_undo_main()
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
	user_ty		*up;
	string_ty	*dd;
	string_ty	*bl;
	int		number_of_errors;

	trace(("new_test_undo_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	wl_zero(&wl);
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_test_undo_usage);
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
				new_test_undo_usage();
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
				new_test_undo_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
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

	/*
	 * It is an error if the change is not in the in_development state.
	 * It is an error if the change is not assigned to the current user.
	 */
	if (cstate_data->state != cstate_state_being_developed)
	{
		change_fatal
		(
			cp,
"this change is in the '%s' state, \
it must be in the 'being developed' state to undo new tests",
			cstate_state_ename(cstate_data->state)
		);
	}
	if (!str_equal(change_developer_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
	"user \"%S\" is not the developer, only user \"%S\" may undo new tests",
			user_name(up),
			change_developer_name(cp)
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
		if
		(
			src_data->action != file_action_create
		||
			(
				src_data->usage != file_usage_test
			&&
				src_data->usage != file_usage_manual_test
			)
		)
		{
			change_error
			(
				cp,
		      "file \"%S\" was not added to this change with -New_Test",
				s1
			);
			++number_of_errors;
		}
	}
	if (number_of_errors)
	{
		change_fatal
		(
			cp,
			"found %d fatal error%s, no new tests removed",
			number_of_errors,
			(number_of_errors == 1 ? "" : "s")
		);
	}

	/*
	 * Remove each file from the development directory,
	 * if it still exists.
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		int		exists;

		s1 = wl.wl_word[j];
		s2 = str_format("%S/%S", dd, s1);
		user_become(up);
		exists = os_exists(s2);
		user_become_undo();
		if (exists && user_delete_file_query(up, s1, 0))
		{
			user_become(up);
			commit_unlink_errok(s2);
			user_become_undo();
		}
		str_free(s2);

		s2 = str_format("%S/%S,D", dd, s1);
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
	for (j = 0; j < wl.wl_nwords; ++j)
		change_src_remove(cp, wl.wl_word[j]);

	/*
	 * the number of files changed, or the version did,
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
		change_verbose(cp, "file \"%S\": new test undo", wl.wl_word[j]);
	wl_free(&wl);
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
new_test_undo()
{
	trace(("new_test_undo()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		new_test_undo_main();
		break;

	case arglex_token_help:
		new_test_undo_help();
		break;

	case arglex_token_list:
		new_test_undo_list();
		break;
	}
	trace((/*{*/"}\n"));
}
