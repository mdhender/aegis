/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to implement remove developer
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <ael.h>
#include <aerd.h>
#include <arglex2.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project_hist.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <str_list.h>


static void remove_developer_usage _((void));

static void
remove_developer_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf(stderr, "usage: %s -Remove_Developer [ <option>... ] <username>...\n", progname);
	fprintf(stderr, "       %s -Remove_Developer -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Remove_Developer -Help\n", progname);
	quit(1);
}


static void remove_developer_help _((void));

static void
remove_developer_help()
{
	help("aerd", remove_developer_usage);
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
				option_needs_name(arglex_token_project, usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, usage);
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
	string_list_ty	wl;
	string_ty	*s1;
	int		j;
	string_ty	*project_name;
	project_ty	*pp;
	user_ty		*up;

	trace(("remove_developer_main()\n{\n"/*}*/));
	string_list_constructor(&wl);
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(remove_developer_usage);
			continue;

		case arglex_token_user:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_user, remove_developer_usage);
			/* fall through... */

		case arglex_token_string:
			s1 = str_from_c(arglex_value.alv_string);
			if (string_list_member(&wl, s1))
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "Name", "%S", s1);
				fatal_intl(scp, i18n("too many user $name"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			string_list_append(&wl, s1);
			str_free(s1);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, remove_developer_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, remove_developer_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(remove_developer_usage);
			break;
		}
		arglex();
	}
	if (!wl.nstrings)
	{
		error_intl(0, i18n("no user names"));
		remove_developer_usage();
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
	 * lock the project for change
	 */
	project_pstate_lock_prepare(pp);
	lock_take();

	/*
	 * check they are allowed to do this
	 */
	if (!project_administrator_query(pp, user_name(up)))
		project_fatal(pp, 0, i18n("not an administrator"));

	/*
	 * check they they are OK users
	 */
	for (j = 0; j < wl.nstrings; ++j)
	{
		user_ty	*candidate;

		candidate = user_symbolic(pp, wl.string[j]);
		if (!project_developer_query(pp, user_name(candidate)))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%S", user_name(candidate));
			project_fatal
			(
				pp,
				scp,
				i18n("user \"$name\" is not a developer")
			);
			/* NOTREACHED */
			sub_context_delete(scp);
		}
		project_developer_remove(pp, user_name(candidate));
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
	for (j = 0; j < wl.nstrings; ++j)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", wl.string[j]);
		project_verbose(pp, scp, i18n("remove developer $name complete"));
		sub_context_delete(scp);
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
