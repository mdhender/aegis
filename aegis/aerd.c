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
 * MANIFEST: functions to implement remove developer
 */

#include <stdio.h>
#include <ac/stdlib.h>

#include <ael.h>
#include <aerd.h>
#include <arglex2.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <user.h>
#include <word.h>


static void remove_developer_usage _((void));

static void
remove_developer_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -Remove_Developer [ <option>... ] <username>...\n", progname);
	fprintf(stderr, "       %s -Remove_Developer -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Remove_Developer -Help\n", progname);
	quit(1);
}


static void remove_developer_help _((void));

static void
remove_developer_help()
{
	static char *text[] =
	{
#include <../man1/aerd.h>
	};

	help(text, SIZEOF(text), remove_developer_usage);
}


static void remove_developer_list _((void (*)(void)));

static void
remove_developer_list(usage)
	void		(*usage)_((void));
{
	string_ty	*project_name;

	trace(("remove_developer_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_developers(project_name, 0);
	trace((/*{*/"}\n"));
}


static void remove_developer_main _((void));

static void
remove_developer_main()
{
	wlist		wl;
	string_ty	*s1;
	pstate		pstate_data;
	int		j;
	string_ty	*project_name;
	project_ty	*pp;
	user_ty		*up;

	trace(("remove_developer_main()\n{\n"/*}*/));
	wl_zero(&wl);
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(remove_developer_usage);
			continue;

		case arglex_token_string:
			s1 = str_from_c(arglex_value.alv_string);
			if (wl_member(&wl, s1))
				fatal("user \"%s\" named more than once", s1->str_text);
			wl_append(&wl, s1);
			str_free(s1);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				remove_developer_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (!wl.wl_nwords)
		fatal("no users named");

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
	 * lock the project for change
	 */
	project_pstate_lock_prepare(pp);
	lock_take();
	pstate_data = project_pstate_get(pp);

	/*
	 * check they are allowed to do this
	 */
	if (!project_administrator_query(pp, user_name(up)))
	{
		project_fatal
		(
			pp,
			"user \"%S\" is not an administrator",
			user_name(up)
		);
	}

	/*
	 * check they they are OK users
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		user_ty	*candidate;

		candidate = user_symbolic(pp, wl.wl_word[j]);
		if (!project_developer_query(pp, user_name(candidate)))
		{
			project_fatal
			(
				pp,
				"user \"%S\" is not a developer",
				user_name(candidate)
			);
		}
		project_developer_delete(pp, user_name(candidate));
		user_free(candidate);
	}

	/*
	 * write out and release lock
	 */
	project_pstate_write(pp);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		project_verbose
		(
			pp,
			"user \"%S\" is no longer a developer",
			wl.wl_word[j]
		);
	}
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
remove_developer()
{
	trace(("remove_developer()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		remove_developer_main();
		break;

	case arglex_token_help:
		remove_developer_help();
		break;

	case arglex_token_list:
		remove_developer_list(remove_developer_usage);
		break;
	}
	trace((/*{*/"}\n"));
}
