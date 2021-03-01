/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to implement develop begin
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <aedb.h>
#include <ael/change/by_state.h>
#include <arglex2.h>
#include <change.h>
#include <col.h>
#include <commit.h>
#include <common.h>
#include <dir.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project_hist.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void develop_begin_usage _((void));

static void
develop_begin_usage()
{
	char		*progname;

	progname = progname_get();
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
	help("aedb", develop_begin_usage);
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
				option_needs_name(arglex_token_project, develop_begin_usage);
			/* fall through... */
		
		case arglex_token_string:
			if (project_name)
				duplicate_option_by_name(arglex_token_project, develop_begin_usage);
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
	cstate_history	history_data;
	string_ty	*devdir;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	pconf		pconf_data;
	string_ty	*usr;
	user_ty		*up2;
	log_style_ty	log_style;

	trace(("develop_begin_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	devdir = 0;
	usr = 0;
	log_style = log_style_create_default;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_begin_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, develop_begin_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, develop_begin_usage);
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

		case arglex_token_directory:
			if (arglex() != arglex_token_string)
				option_needs_dir(arglex_token_directory, develop_begin_usage);
			if (devdir)
				duplicate_option_by_name(arglex_token_directory, develop_begin_usage);
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
				option_needs_name(arglex_token_project, develop_begin_usage);
			/* fall through... */
		
		case arglex_token_string:
			if (project_name)
				duplicate_option_by_name(arglex_token_project, develop_begin_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_user:
			if (usr)
				duplicate_option(develop_begin_usage);
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_user, develop_begin_usage);
			usr = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (log_style == log_style_none)
				duplicate_option(develop_begin_usage);
			log_style = log_style_none;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(develop_begin_usage);
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
			project_fatal(pp, 0, i18n("not an administrator"));
	}
	else
	{
		up = user_executing(pp);
		up2 = 0;
	}

	/*
	 * Make sure the tests don't go too fast.
	 */
	os_throttle();

	/*
	 * locate change data
	 *
	 * The change number must be given on the command line,
	 * even if there is only one appropriate change.
	 * The is the "least surprizes" principle at work,
	 * even though we could sometimes work this out for ourself.
	 */
	if (!change_number)
		fatal_intl(0, i18n("no change number"));
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

	/*
	 * Extract the appropriate row of the change table.
	 * It is an error if the change is not in the
	 * undevelop_begined state.
	 */
	if (cstate_data->state != cstate_state_awaiting_development)
		change_fatal(cp, 0, i18n("bad db state"));
	if (!project_developer_query(pp, user_name(up)))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		if (up2)
		{
			sub_var_set(scp, "User", "%S", user_name(up));
			sub_var_optional(scp, "User");
			sub_var_override(scp, "User");
		}
		project_fatal(pp, scp, i18n("not a developer"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * Work out the development directory.
	 *
	 * (Do this before the state advances to being developed,
	 * otherwise it tries to find the config file in the as-yet
	 * non-existant development directory.)
	 */
	if (!devdir)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		devdir = change_development_directory_template(cp, up);
		sub_var_set(scp, "File_Name", "%S", devdir);
		change_verbose(cp, scp, i18n("development directory \"$filename\""));
		sub_context_delete(scp);
	}
	change_development_directory_set(cp, devdir);

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
	 * Create the development directory.
	 */
	user_become(up);
	os_mkdir(devdir, 02755);
	undo_rmdir_errok(devdir);
	user_become_undo();

	/*
	 * Update user change table to include this change in the list of
	 * changes being developed by this user.
	 */
	user_own_add(up, project_name_get(pp), change_number);

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
	log_open(change_logfile_get(cp), up, log_style);
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
		change_create_symlinks_to_baseline(cp, pp, up, 0);

	/*
	 * verbose success message
	 */
	change_verbose(cp, 0, i18n("develop begin complete"));
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
