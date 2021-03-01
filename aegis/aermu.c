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
 * MANIFEST: functions to implement remove file undo
 */

#include <stdio.h>
#include <ac/stdlib.h>

#include <ael.h>
#include <aermu.h>
#include <arglex2.h>
#include <change.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <user.h>
#include <word.h>


/*
 * NAME
 *	remove_file_undo_usage
 *
 * SYNOPSIS
 *	void remove_file_undo_usage(void);
 *
 * DESCRIPTION
 *	The remove_file_undo_usage function is used to
 *	tell the user how to use the 'aegis -ReMove_file_Undo' command.
 */

static void remove_file_undo_usage _((void));

static void
remove_file_undo_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf
	(
		stderr,
		"usage: %s -ReMove_file_Undo <filename>... [ <option>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -ReMove_file_Undo -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -ReMove_file_Undo -Help\n", progname);
	quit(1);
}


/*
 * NAME
 *	remove_file_undo_help
 *
 * SYNOPSIS
 *	void remove_file_undo_help(void);
 *
 * DESCRIPTION
 *	The remove_file_undo_help function is used to
 *	describe the 'aegis -ReMove_file_undo' command to the user.
 */

static void remove_file_undo_help _((void));

static void
remove_file_undo_help()
{
	static char *text[] =
	{
#include <../man1/aermu.h>
	};

	help(text, SIZEOF(text), remove_file_undo_usage);
}


/*
 * NAME
 *	remove_file_undo_list
 *
 * SYNOPSIS
 *	void remove_file_undo_list(void);
 *
 * DESCRIPTION
 *	The remove_file_undo_list function is used to
 *	list the file the user may wish to remove from the change
 *	as a deletion.  All relevant change files are listed.
 */

static void remove_file_undo_list _((void));

static void
remove_file_undo_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("remove_file_undo_list()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(remove_file_undo_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				remove_file_undo_usage();
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
				remove_file_undo_usage();
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


/*
 * NAME
 *	remove_undo_main
 *
 * SYNOPSIS
 *	void remove_undo_main(void);
 *
 * DESCRIPTION
 *	The remove_undo_main function is used to
 *	remove a file from a change as a deletion.
 *
 *	The names of the relevant files are gleaned from the command line.
 */

static void remove_file_undo_main _((void));

static void
remove_file_undo_main()
{
	wlist		wl;
	string_ty	*s1;
	string_ty	*s2;
	int		j;
	cstate		cstate_data;
	pstate		pstate_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;
	string_ty	*dd;
	string_ty	*bl;
	int		number_of_errors;

	trace(("remove_file_undo_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	wl_zero(&wl);
	nolog = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(remove_file_undo_usage);
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
			str_free(s2);
			str_free(s1);
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				remove_file_undo_usage();
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
				remove_file_undo_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (nolog)
			{
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
	if (!wl.wl_nwords)
		fatal("no files specified");

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
	 * take the locks and read the change state
	 */
	change_cstate_lock_prepare(cp);
	lock_take();
	pstate_data = project_pstate_get(pp);
	cstate_data = change_cstate_get(cp);

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
it must be in the 'being developed' state to undo remove files",
			cstate_state_ename(cstate_data->state)
		);
	}
	if (!str_equal(change_developer_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
     "user \"%S\" is not the developer, only user \"%S\" may undo remove files",
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
	 * ensure that each file is part of the change
	 */
	number_of_errors = 0;
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		cstate_src	c_src_data;

		s1 = wl.wl_word[j];
		c_src_data = change_src_find(cp, s1);
		if (!c_src_data)
		{
			c_src_data = change_src_find_fuzzy(cp, s1);
			if (c_src_data)
			{
				change_error
				(
					cp,
		       "file \"%S\" not in change, closest was the \"%S\" file",
					s1,
					c_src_data->file_name
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
		if (c_src_data->action != file_action_remove)
		{
			change_error
			(
				cp,
		   "file \"%S\" was not added to this change with -ReMove_file",
				s1
			);
			++number_of_errors;
			continue;
		}
		change_src_remove(cp, s1);
	}
	if (number_of_errors)
	{
		change_fatal
		(
			cp,
			"found %d fatal error%s, no removed files were removed",
			number_of_errors,
			(number_of_errors == 1 ? "" : "s")
		);
	}

	/*
	 * Remove the difference files,
	 * if they exist.
	 */
	user_become(up);
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		s1 = wl.wl_word[j];
		s2 = str_format("%S/%S,D", dd, s1);
		if (os_exists(s2))
			commit_unlink_errok(s2);
		str_free(s2);
	}
	user_become_undo();

	/*
	 * the number of files changed, or the version did,
	 * so stomp on the validation fields.
	 */
	change_build_times_clear(cp);

	/*
	 * write the data and release the lock
	 */
	change_cstate_write(cp);
	commit();
	lock_release();

	/*
	 * run the change file command
	 */
	if (!nolog)
		log_open(change_logfile_get(cp), up, log_style_append);
	change_run_change_file_command(cp, &wl, up);

	/*
	 * verbose success message
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		change_verbose
		(
			cp,
			"file \"%S\" remove file undo",
			wl.wl_word[j]
		);
	}
	wl_free(&wl);
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	remove_file_undo
 *
 * SYNOPSIS
 *	void remove_file_undo(void);
 *
 * DESCRIPTION
 *	The remove_file_undo function is used to
 *	dispatch the 'aegis -ReMove_file_Undo' command to the relevant
 *	function to do it's work.
 */

void
remove_file_undo()
{
	trace(("remove_file_undo()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		remove_file_undo_main();
		break;

	case arglex_token_help:
		remove_file_undo_help();
		break;

	case arglex_token_list:
		remove_file_undo_list();
		break;
	}
	trace((/*{*/"}\n"));
}
