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
 * MANIFEST: functions to implement new test
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>

#include <ael.h>
#include <aent.h>
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


static void new_test_usage _((void));

static void
new_test_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -New_Test [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Test -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Test -Help\n", progname);
	quit(1);
}


static void new_test_help _((void));

static void
new_test_help()
{
	static char *text[] =
	{
#include <../man1/aent.h>
	};

	help(text, SIZEOF(text), new_test_usage);
}


static void new_test_list _((void));

static void
new_test_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("new_test_list()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_test_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				new_test_usage();
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
				new_test_usage();
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


static void new_test_main _((void));

static void
new_test_main()
{
	cstate		cstate_data;
	pstate		pstate_data;
	string_ty	*s1;
	string_ty	*s2;
	string_ty	*dd;
	cstate_src	src_data;
	int		manual_flag;
	int		automatic_flag;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;

	trace(("new_test_main()\n{\n"/*}*/));
	manual_flag = 0;
	automatic_flag = 0;
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_test_usage);
			continue;

		case arglex_token_manual:
			if (manual_flag)
			{
				duplicate:
				fatal
				(
					"duplicate %s option",
					arglex_value.alv_string
				);
			}
			manual_flag = 1;
			break;

		case arglex_token_automatic:
			if (automatic_flag)
				goto duplicate;
			automatic_flag = 1;
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				new_test_usage();
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
				new_test_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (automatic_flag && manual_flag)
		fatal("may not specify both -MANual and -AUTOmatic");
	if (!manual_flag && !automatic_flag)
		automatic_flag = 1;

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
	 * lock the change state file
	 * and the project state file for the test number
	 */
	project_pstate_lock_prepare(pp);
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
it must be in the 'being developed' state to create a new test with it",
			cstate_state_ename(cstate_data->state)
		);
	}
	if (!str_equal(change_developer_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
"user \"%S\" is not the developer, only user \"%S\" may add a new test",
			user_name(up),
			change_developer_name(cp)
		);
	}

	/*
	 * Create each file in the development directory.
	 * Create any necessary directories along the way.
	 */
	dd = change_development_directory_get(cp, 1);
	s1 =
		str_format
		(
			"test/%2.2ld/t%4.4ld%s.sh",
			pstate_data->next_test_number / 100,
			pstate_data->next_test_number,
			(manual_flag ? "m" : "a")
		);
	pstate_data->next_test_number++,
	user_become(up);
	os_mkdir_between(dd, s1, 02755);
	s2 = str_format("%S/%S", dd, s1);
	if (!os_exists(s2))
	{
		int		fd;
		string_ty	*template;

		os_become_undo();
		template = change_file_template(cp, s1);
		user_become(up);
		fd = glue_creat(s2->str_text, 0666);
		if (fd < 0)
			nfatal("create(\"%s\")", s2->str_text);
		if (template)
		{
			glue_write
			(
				fd,
				template->str_text,
				template->str_length
			);
			str_free(template);
		}
		glue_close(fd);
		os_chmod(s2, 0644 & ~change_umask(cp));
	}
	str_free(s2);
	os_become_undo();

	/*
	 * Add the file to the change,
	 * and write it back out.
	 */
	src_data = change_src_new(cp);
	src_data->file_name = s1;
	src_data->action = file_action_create;
	src_data->usage =
		(manual_flag ? file_usage_manual_test : file_usage_test);

	/*
	 * the number of files changed, or the version did,
	 * so stomp on the validation fields.
	 */
	change_build_times_clear(cp);

	/*
	 * The change now has at least one test,
	 * so cancel any testing exemption.
	 */
	cstate_data->test_exempt = 0;
	if (change_number > 1)
		cstate_data->test_baseline_exempt = 0;

	/*
	 * release the locks
	 */
	project_pstate_write(pp);
	change_cstate_write(cp);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	change_verbose(cp, "file \"%S\" new test", s1);
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
new_test()
{
	trace(("new_test()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		new_test_main();
		break;

	case arglex_token_help:
		new_test_help();
		break;

	case arglex_token_list:
		new_test_list();
		break;
	}
	trace((/*{*/"}\n"));
}
