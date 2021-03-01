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
 * MANIFEST: functions to implement new integrator
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <ael/project/integrators.h>
#include <aeni.h>
#include <arglex2.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project_hist.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


static void new_integrator_usage _((void));

static void
new_integrator_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf(stderr, "usage: %s -New_Integrator <username>... [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Integrator -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Integrator -Help\n", progname);
	quit(1);
}


static void new_integrator_help _((void));

static void
new_integrator_help()
{
	help("aeni", new_integrator_usage);
}


static void new_integrator_list _((void (*)(void)));

static void
new_integrator_list(usage)
	void		(*usage)_((void));
{
	string_ty	*project_name;

	trace(("new_integrator_list()\n{\n"/*}*/));
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
	list_integrators(project_name, 0);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void new_integrator_main _((void));

static void
new_integrator_main()
{
	string_list_ty	wl;
	string_ty	*s1;
	int		j;
	string_ty	*project_name;
	project_ty	*pp;
	user_ty		*up;

	trace(("new_integrator_main()\n{\n"/*}*/));
	string_list_constructor(&wl);
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_integrator_usage);
			continue;

		case arglex_token_user:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_user, new_integrator_usage);
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
				option_needs_name(arglex_token_project, new_integrator_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, new_integrator_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(new_integrator_usage);
			break;
		}
		arglex();
	}
	if (!wl.nstrings)
	{
		error_intl(0, i18n("no user names"));
		new_integrator_usage();
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
		user_ty		*candidate;

		/*
		 * make sure the user isn't already there
		 */
		candidate = user_symbolic(pp, wl.string[j]);
		if (project_integrator_query(pp, user_name(candidate)))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%S", user_name(candidate));
			project_fatal(pp, scp, i18n("$name already integrator"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}

		/*
		 * make sure the user exists
		 *	(should we check s/he is in the project's group?)
		 * this is to avoid security holes
		 */
		if (!user_uid_check(user_name(candidate)))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%S", user_name(candidate));
			fatal_intl(scp, i18n("user \"$name\" is too privileged"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}

		/*
		 * add it to the list
		 */
		project_integrator_add(pp, user_name(candidate));
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
		project_verbose(pp, scp, i18n("new integrator $name complete"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
new_integrator()
{
	trace(("new_integrator()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		new_integrator_main();
		break;

	case arglex_token_help:
		new_integrator_help();
		break;

	case arglex_token_list:
		new_integrator_list(new_integrator_usage);
		break;
	}
	trace((/*{*/"}\n"));
}
