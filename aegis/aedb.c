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
 * MANIFEST: functions to implement develop begin
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <aedb.h>
#include <ael.h>
#include <arglex2.h>
#include <change.h>
#include <col.h>
#include <commit.h>
#include <common.h>
#include <dir.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void develop_begin_usage _((void));

static void
develop_begin_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf
	(
		stderr,
		"usage: %s -Develop_Begin <change_number> [ <option>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -Develop_Begin -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -Develop_Begin -Help\n", progname);
	quit(1);
}


static void develop_begin_help _((void));

static void
develop_begin_help()
{
	static char *text[] =
	{
#include <../man1/aedb.h>
	};

	help(text, SIZEOF(text), develop_begin_usage);
}


static void develop_begin_list _((void));

static void
develop_begin_list()
{
	string_ty	*project_name;

	trace(("develop_begin_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_begin_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				develop_begin_usage();
			/* fall through... */
		
		case arglex_token_string:
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_changes_in_state_mask
	(
		project_name,
		1 << cstate_state_awaiting_development
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void develop_begin_main _((void));

static void
develop_begin_main()
{
	cstate		cstate_data;
	pstate		pstate_data;
	cstate_history	history_data;
	string_ty	*devdir;
	string_ty	*s2;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	pconf		pconf_data;
	string_ty	*usr;
	user_ty		*up2;

	trace(("develop_begin_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	devdir = 0;
	usr = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_begin_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				develop_begin_usage();
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				fatal("duplicate -Change option");
			change_number = arglex_value.alv_number;
			if (change_number < 1)
				fatal("change %ld out of range", change_number);
			break;

		case arglex_token_directory:
			if (arglex() != arglex_token_string)
				develop_begin_usage();
			if (devdir)
				fatal("duplicate -DIRectory option");
			/*
			 * To cope with automounters, directories are stored as
			 * given, or are derived from the home directory in the
			 * passwd file.  Within aegis, pathnames have their
			 * symbolic links resolved, and any comparison of paths
			 * is done on this "system idea" of the pathname.
			 */
			devdir = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				develop_begin_usage();
			/* fall through... */
		
		case arglex_token_string:
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_user:
			if (usr)
				fatal("duplicate -User option");
			if (arglex() != arglex_token_string)
				develop_begin_usage();
			usr = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
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
	 *
	 *	up = user to own the change
	 *	up2 = administrator forcing
	 */
	if (usr)
	{
		up = user_symbolic(pp, usr);
		up2 = user_executing(pp);
		if (up == up2)
		{
			user_free(up2);
			up2 = 0;
		}
		else if (!project_administrator_query(pp, user_name(up2)))
		{
			project_fatal
			(
				pp,
				"user \"%S\" is not an administrator",
				user_name(up2)
			);
		}
	}
	else
	{
		up = user_executing(pp);
		up2 = 0;
	}

	/*
	 * locate change data
	 *
	 * The change number must be given on the command line,
	 * even if there is only one appropriate change.
	 * The is the "least surprizes" principle at work,
	 * even though we could sometimes work this out for ourself.
	 */
	if (!change_number)
		fatal("the change number must be stated explicitly");
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	/*
	 * Take an advisory write lock on the appropriate row of the change
	 * table.  Take an advisory write lock on the appropriate row of the
	 * user table.  The user table row may need to be created.
	 * Block while can't get both simultaneously.
	 */
	user_ustate_lock_prepare(up);
	change_cstate_lock_prepare(cp);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * Extract the appropriate row of the change table.
	 * It is an error if the change is not in the
	 * undevelop_begined state.
	 */
	if (cstate_data->state != cstate_state_awaiting_development)
	{
		change_fatal
		(
			cp,
"this change is in the '%s' state, \
it must be in the 'awaiting development' state to begin development",
			cstate_state_ename(cstate_data->state)
		);
	}
	if (!project_developer_query(pp, user_name(up)))
	{
		project_fatal
		(
			pp,
			"user \"%S\" is not a developer",
			user_name(up)
		);
	}

	/*
	 * Set the change data to reflect the current user
	 * as developer and move it to the in-development state.
	 * Append another entry to the change history.
	 */
	cstate_data->state = cstate_state_being_developed;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_develop_begin;
	if (up2)
	{
		history_data->why =
			str_format
			(
				"Forced by administrator \"%S\".",
				user_name(up2)
			);
	}

	/*
	 * Update user change table to include this change in the list of
	 * changes being developed by this user.
	 */
	user_own_add(up, project_name_get(pp), change_number);

	/*
	 * Create the change directory.
	 */
	if (!devdir)
	{
		unsigned long	k;
		int		max;
		string_ty	*pn;

		/*
		 * If the user did not give the directory to use,
		 * we must construct one.
		 * The length is limited by the available filename
		 * length limit, trim the project name if necessary.
		 */
		pn = project_name_get(pp);
		s2 = user_default_development_directory(up);
		assert(s2);
		os_become_orig();
		max = os_pathconf_name_max(s2);
		os_become_undo();
		for (k = 0;; ++k)
		{
			char		suffix[30];
			char		*tp;
			unsigned long	n;
			int		len;
			int		exists;

			tp = suffix;
			*tp++ = '.';
			n = k;
			for (;;)
			{
				*tp++ = (n & 15) + 'C';
				n >>= 4;
				if (!n)
					break;
			}
			sprintf(tp, "%3.3ld", change_number);

			len = strlen(suffix);
			if (len > max)
			{
				/* unlikely in the extreme */
				len = max - 1;
				suffix[len] = 0;
			}
			len = max - len;
			if (len > pn->str_length)
				len = pn->str_length;
			devdir = str_format("%S/%.*S%s", s2, len, pn, suffix);
			os_become_orig();
			exists = os_exists(devdir);
			os_become_undo();
			if (!exists)
				break;
			str_free(devdir);
		}
		str_free(s2);
		change_verbose(cp, "development directory \"%S\"", devdir);
	}
	change_development_directory_set(cp, devdir);

	/*
	 * Create the development directory.
	 */
	user_become(up);
	os_mkdir(devdir, 02755);
	undo_rmdir_errok(devdir);
	user_become_undo();

	/*
	 * Clear the time fields.
	 */
	change_build_times_clear(cp);

	/*
	 * Update change table row (and change history table).
	 * Update user table row.
	 * Release advisory write locks.
	 */
	change_cstate_write(cp);
	user_ustate_write(up);
	commit();
	lock_release();

	/*
	 * run the develop begin command
	 */
	change_run_develop_begin_command(cp, up);

	/*
	 * run the forced develop begin notify command
	 */
	if (up2)
		change_run_forced_develop_begin_notify_command(cp, up2);

	/*
	 * if symlinks are being used to pander to dumb DMT,
	 * and they are not removed after each build,
	 * create them now, rather than waiting for the first build.
	 * This will present a more uniform interface to the developer.
	 */
	pconf_data = change_pconf_get(cp, 0);
	if
	(
		pconf_data->create_symlinks_before_build
	&&
		!pconf_data->remove_symlinks_after_build
	)
		change_create_symlinks_to_baseline(cp, up);

	/*
	 * verbose success message
	 */
	change_verbose
	(
		cp,
		"user \"%S\" has begun development",
		user_name(up)
	);
	change_free(cp);
	project_free(pp);
	user_free(up);
	if (up2)
		user_free(up2);
	trace((/*{*/"}\n"));
}


void
develop_begin()
{
	trace(("develop_begin()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		develop_begin_main();
		break;

	case arglex_token_help:
		develop_begin_help();
		break;

	case arglex_token_list:
		develop_begin_list();
		break;
	}
	trace((/*{*/"}\n"));
}
