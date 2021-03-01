/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1996, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to implement the 'aegis -Delta_Name' command
 */

#include <ac/stdio.h>

#include <aedn.h>
#include <ael/project/history.h>
#include <arglex2.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <progname.h>
#include <project.h>
#include <project_hist.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <zero.h>


static void delta_name_usage _((void));

static void
delta_name_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf
	(
		stderr,
	"usage: %s -Delta_Name [ <option>... ][ <delta_number> ] <string>...\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -Delta_Name -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -Delta_Name -Help\n", progname);
	quit(1);
}


static void delta_name_help _((void));

static void
delta_name_help()
{
	help("aedn", delta_name_usage);
}


static void delta_name_list _((void));

static void
delta_name_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("delta_name_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(delta_name_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, delta_name_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, delta_name_usage);
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
				option_needs_name(arglex_token_project, delta_name_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, delta_name_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_project_history(project_name, change_number);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void delta_name_main _((void));

static void
delta_name_main()
{
	sub_context_ty	*scp;
	string_ty	*project_name;
	long		delta_number;
	string_ty	*delta_name;
	int		stomp;
	project_ty	*pp;
	user_ty		*up;

	trace(("delta_name_main()\n{\n"/*}*/));
	project_name = 0;
	delta_number = 0;
	delta_name = 0;
	stomp = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(delta_name_usage);
			continue;

		case arglex_token_overwriting:
			if (stomp)
				duplicate_option(delta_name_usage);
			stomp = 1;
			break;

		case arglex_token_string:
			if (delta_name)
			{
				error_intl(0, i18n("too many delta names"));
				delta_name_usage();
			}
			delta_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_number:
			if (delta_number)
			{
				error_intl(0, i18n("too many delta numbers"));
				delta_name_usage();
			}
			delta_number = arglex_value.alv_number;
			if (delta_number < 1)
			{
				scp = sub_context_new();
				sub_var_set(scp, "Number", "%ld", delta_number);
				fatal_intl(scp, i18n("delta $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (project_name)
				duplicate_option(delta_name_usage);
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, delta_name_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(delta_name_usage);
			break;
		}
		arglex();
	}
	if (!delta_name)
	{
		error_intl(0, i18n("no delta name"));
		delta_name_usage();
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
	 * lock the project file
	 */
	project_pstate_lock_prepare(pp);
	lock_take();

	/*
	 * it is an error if the user is not a project administrator
	 */
	if (!project_administrator_query(pp, user_name(up)))
		project_fatal(pp, 0, i18n("not an administrator"));

	/*
	 * it is an error if the delta does not exist
	 */
	if (delta_number)
	{
		if (!project_history_delta_validate(pp, delta_number))
		{
			scp = sub_context_new();
			sub_var_set(scp, "Number", "%ld", delta_number);
			project_fatal(pp, scp, i18n("no delta $number"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
	}
	else
	{
		delta_number = project_history_delta_latest(pp);
		if (delta_number <= 0)
			project_fatal(pp, 0, i18n("no delta yet"));
	}

	if (!stomp)
	{
		long		other;

		other = project_history_delta_by_name(pp, delta_name, 1);
		if (other && other != delta_number)
		{
			scp = sub_context_new();
			sub_var_set(scp, "Name", "%S", delta_name);
			sub_var_set(scp, "Number", "%ld", delta_number);
			sub_var_optional(scp, "Number");
			sub_var_set(scp, "Other", "%ld", other);
			sub_var_optional(scp, "Other");
			project_fatal(pp, scp, i18n("delta $name in use"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
	}
	project_history_delta_name_delete(pp, delta_name);

	/*
	 * add the name to the selected history entry
	 */
	project_history_delta_name_add(pp, delta_number, delta_name);

	/*
	 * release the locks
	 */
	project_pstate_write(pp);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	scp = sub_context_new();
	sub_var_set(scp, "Name", "%S", delta_name);
	sub_var_optional(scp, "Name");
	sub_var_set(scp, "Number", "%ld", delta_number);
	sub_var_optional(scp, "Number");
	project_verbose(pp, scp, i18n("delta name complete"));
	sub_context_delete(scp);
	project_free(pp);
	user_free(up);
	str_free(delta_name);
	trace((/*{*/"}\n"));
}


void
delta_name_assignment()
{
	trace(("delta_name()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		delta_name_main();
		break;

	case arglex_token_help:
		delta_name_help();
		break;

	case arglex_token_list:
		delta_name_list();
		break;
	}
	trace((/*{*/"}\n"));
}
