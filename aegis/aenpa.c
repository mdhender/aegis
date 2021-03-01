/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to create new project aliases
 */

#include <ac/stdio.h>

#include <ael/project/aliases.h>
#include <aenpa.h>
#include <arglex2.h>
#include <commit.h>
#include <error.h>
#include <gonzo.h>
#include <help.h>
#include <lock.h>
#include <progname.h>
#include <project.h>
#include <project_hist.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


static void project_alias_create_usage _((void));

static void
project_alias_create_usage()
{
	char	*progname;

	progname = progname_get();
	fprintf
	(
		stderr,
		"usage: %s -New_Project_Alias <name> [ <option>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -New_Project_Alias -List [ <option>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -New_Project_Alias -Help\n",
		progname
	);
	quit(1);
}


static void project_alias_create_help _((void));

static void
project_alias_create_help()
{
	help("aenpa", project_alias_create_usage);
}


static void project_alias_create_list _((void));

static void
project_alias_create_list()
{
	arglex();
	while (arglex_token != arglex_token_eoln)
		generic_argument(project_alias_create_usage);
	list_project_aliases(0, 0);
}


static void project_alias_create_main _((void));

static void
project_alias_create_main()
{
	sub_context_ty	*scp;
	string_ty	*project_name[2];
	int		project_name_count;
	project_ty	*pp;
	user_ty		*up;

	trace(("project_alias_create_main()\n{\n"));
	arglex();
	project_name_count = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(project_alias_create_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, project_alias_create_usage);
			/* fall through... */

		case arglex_token_string:
			if (project_name_count >= 2)
				fatal_intl(0, i18n("too many proj name"));
			project_name[project_name_count++] =
				str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(project_alias_create_usage);
			break;
		}
		arglex();
	}
	if (project_name_count != 2)
		fatal_intl(0, i18n("no project name"));

	/*
	 * locate OLD project data
	 */
	pp = project_alloc(project_name[0]);
	project_bind_existing(pp);

	/*
	 * locate user data
	 */
	up = user_executing(pp);

	/*
	 * it is an error if the current user is not an administrator
	 * of the old project.
	 */
	if (!project_administrator_query(pp, user_name(up)))
		project_fatal(pp, 0, i18n("not an administrator"));

	/*
	 * Make sure the project alias is acceptable.
	 */
	if
	(
		!project_name_ok(project_name[1])
	||
		!gonzo_alias_acceptable(project_name[1])
	)
	{
		scp = sub_context_new();
		sub_var_set_string(scp, "Name", project_name[1]);
		fatal_intl(scp, i18n("bad alias $name"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * take the relevant locks
	 */
	gonzo_gstate_lock_prepare_new();
	lock_take();

	/* 
	 * add the new alias
	 *	it is an error if it is already in use
	 */
	if (gonzo_alias_to_actual(project_name[1]))
		fatal_project_alias_exists(project_name[1]);
	if (gonzo_project_home_path_from_name(project_name[1]))
	{
		scp = sub_context_new();
		sub_var_set_string(scp, "Name", project_name[1]);
		fatal_intl(scp, i18n("project $name exists"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	gonzo_alias_add(pp, project_name[1]);
	gonzo_gstate_write();

	/*
	 * release locks
	 */
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	scp = sub_context_new();
	sub_var_set_string(scp, "Name", project_name[1]);
	project_verbose(pp, scp, i18n("new alias $name complete"));
	sub_context_delete(scp);
	str_free(project_name[0]);
	project_free(pp);
	str_free(project_name[1]);
	user_free(up);
	trace(("}\n"));
}


void
project_alias_create()
{
	static arglex_dispatch_ty dispatch[] =
	{
		{ arglex_token_help,		project_alias_create_help, },
		{ arglex_token_list,		project_alias_create_list, },
	};

	trace(("project_alias_create()\n{\n"));
	arglex_dispatch(dispatch, SIZEOF(dispatch), project_alias_create_main);
	trace(("}\n"));
}
