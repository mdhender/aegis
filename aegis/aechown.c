/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995 Peter Miller;
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
 * MANIFEST: functions to implement the 'aegis -Change_Owner' command
 */

#include <stdio.h>
#include <ac/string.h>

#include <aechown.h>
#include <ael.h>
#include <arglex2.h>
#include <error.h>
#include <change.h>
#include <commit.h>
#include <cstate.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <pstate.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void change_owner_usage _((void));

static void
change_owner_usage()
{
	char	*progname;

	progname = option_progname_get();
	fprintf
	(
		stderr,
		"usage: %s -Change_Owner [ <option>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -Change_Owner -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -Change_Owner -Help\n", progname);
	quit(1);
}


static void change_owner_help _((void));

static void
change_owner_help()
{
	static char *text[] =
	{
#include <../man1/aechown.h>
	};

	help(text, SIZEOF(text), change_owner_usage);
}


static void change_owner_list _((void));

static void
change_owner_list()
{
	string_ty	*project_name;

	trace(("change_owner_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(change_owner_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				change_owner_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_changes(project_name, 1 << cstate_state_being_developed);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void change_owner_main _((void));

static void
change_owner_main()
{
	string_ty	*project_name;
	long		change_number;
	project_ty	*pp;
	user_ty		*up;
	user_ty		*up1;
	user_ty		*up2;
	change_ty	*cp;
	pstate		pstate_data;
	cstate		cstate_data;
	cstate_history	history_data;
	string_ty	*usr;
	string_ty	*devdir;
	string_ty	*old_dd;
	size_t		j;
	pconf		pconf_data;

	trace(("change_owner_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	usr = 0;
	devdir = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(change_owner_usage);
			continue;

		case arglex_token_keep:
		case arglex_token_interactive:
		case arglex_token_no_keep:
			user_delete_file_argument();
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				change_owner_usage();
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				fatal("duplicate -Change option");
			change_number = arglex_value.alv_number;
			if (change_number < 1)
				fatal("change %ld out of range", change_number);
			break;

		case arglex_token_project:
			if (project_name)
				fatal("duplicate -Project option");
			if (arglex() != arglex_token_string)
				change_owner_usage();
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_directory:
			if (devdir)
				fatal("duplicate -Directory option");
			if (arglex() != arglex_token_string)
				change_owner_usage();
			devdir = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_user:
			if (arglex() != arglex_token_string)
				change_owner_usage();
			/* fall through... */

		case arglex_token_string:
			if (usr)
				fatal("duplicate -User option");
			usr = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (!change_number)
		fatal("no change number given");
	if (!usr)
		fatal("no user name given");

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * it is an error if the named user is not a developer
	 */
	if (!project_developer_query(pp, usr))
		project_fatal(pp, "user \"%S\" is not a developer", usr);

	/*
	 * locate user data
	 */
	up = user_executing(pp);

	/*
	 * It is an error if the executing user is not a project administrator
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
	 * locate change data
	 */
	assert(change_number);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	/*
	 * It is an error if the change is not in the 'being developed' state
	 */
	trace(("mark\n"));
	cstate_data = change_cstate_get(cp);
	if (cstate_data->state != cstate_state_being_developed)
	{
		change_fatal
		(
			cp,
"this change is in the '%s' state, \
it must be in the 'being developed' state to change its owner",
			cstate_state_ename(cstate_data->state)
		);
	}
	if (str_equal(change_developer_name(cp), usr))
	{
		change_verbose
		(
			cp,
"warning: no need to change owner, \
already being developed by user \"%S\", \
a new development directory will be constructed",
			usr
		);
	}
	up1 = user_symbolic(pp, change_developer_name(cp));
	trace(("up1 = %08lx\n", up1));
	up2 = user_symbolic(pp, usr);
	trace(("up2 = %08lx\n", up2));

	/*
	 * Take an advisory write lock on the appropriate row of the change
	 * table.  Take an advisory write lock on the appropriate row of the
	 * user table.  Block until can get both simultaneously.
	 */
	trace(("mark\n"));
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	user_ustate_lock_prepare(up1);
	user_ustate_lock_prepare(up2);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * These could have changed, check again:
	 * It is an error if the change is not in the being developed state.
	 * It is an error if the change is already being developed by the
	 * named user.
	 */
	if (cstate_data->state != cstate_state_being_developed)
	{
		change_fatal
		(
			cp,
"this change is in the '%s' state, \
it must be in the 'being developed' state to change its owner",
			cstate_state_ename(cstate_data->state)
		);
	}
	if (!str_equal(change_developer_name(cp), user_name(up1)))
		change_fatal(cp, "sync error, try again");

	/*
	 * add to history for state change
	 */
	trace(("mark\n"));
	history_data = change_history_new(cp, up1);
	history_data->what = cstate_history_what_develop_begin_undo;
	history_data->why =
		str_format("Forced by administrator \"%S\".", user_name(up));
	history_data = change_history_new(cp, up2);
	history_data->what = cstate_history_what_develop_begin;
	history_data->why =
		str_format("Forced by administrator \"%S\".", user_name(up));

	/*
	 * Clear the build-time field.
	 * Clear the test-time field.
	 * Clear the test-baseline-time field.
	 * Clear the src field.
	 */
	change_build_times_clear(cp);

	/*
	 * Remove the change from the list of assigned changes in the user
	 * change table (in the user row).
	 */
	trace(("mark\n"));
	user_own_remove(up1, project_name_get(pp), change_number);
	user_own_add(up2, project_name_get(pp), change_number);

	/*
	 * Create the change directory.
	 */
	trace(("mark\n"));
	if (!devdir)
	{
		unsigned long	k;
		int		max;
		string_ty	*pn;
		string_ty	*s2;

		/*
		 * If the user did not give the directory to use,
		 * we must construct one.
		 * The length is limited by the available filename
		 * length limit, trim the project name if necessary.
		 */
		pn = project_name_get(pp);
		s2 = user_default_development_directory(up2);
		assert(s2);
		user_become(up2);
		max = os_pathconf_name_max(s2);
		user_become_undo();
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
	trace(("mark\n"));
	assert(cstate_data->development_directory);
	old_dd = cstate_data->development_directory;
	cstate_data->development_directory = devdir;

	/*
	 * Create the development directory.
	 */
	trace(("mark\n"));
	user_become(up2);
	os_mkdir(devdir, 02755);
	undo_rmdir_errok(devdir);
	user_become_undo();

	/*
	 * copy change files across
	 *	(even the removed files)
	 */
	change_verbose(cp, "copy change source files");
	trace(("mark\n"));
	user_become(up2);
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		string_ty	*s1;
		cstate_src	src_data;

		/*
		 * copy the file across
		 */
		src_data = cstate_data->src->list[j];
		s1 = str_format("%S/%S", old_dd, src_data->file_name);
		if (os_exists(s1))
		{
			string_ty	*s2;

			s2 = str_format("%S/%S", devdir, src_data->file_name);
			os_mkdir_between(devdir, src_data->file_name, 02755);
			copy_whole_file(s1, s2, 0);
			str_free(s2);
		}
		str_free(s1);

		/*
		 * clear the diff time
		 */
		src_data->diff_time = 0;
		src_data->diff_file_time = 0;
	}
	user_become_undo();

	/*
	 * remove the old development directory
	 */
	trace(("mark\n"));
	if (user_delete_file_query(up, old_dd, 1))
	{
		change_verbose(cp, "remove old development directory");
		user_become(up1);
		commit_rmdir_tree_errok(old_dd);
		user_become_undo();
	}
	str_free(old_dd);

	/*
	 * Write the change table row.
	 * Write the user table rows.
	 * Release advisory locks.
	 */
	trace(("mark\n"));
	change_cstate_write(cp);
	project_pstate_write(pp);
	user_ustate_write(up1);
	user_ustate_write(up2);
	commit();
	lock_release();

	/*
	 * run the develop begin command
	 */
	trace(("mark\n"));
	change_run_develop_begin_command(cp, up2);
	change_run_forced_develop_begin_notify_command(cp, up);

	/*
	 * if symlinks are being used to pander to dumb DMT,
	 * and they are not removed after each build,
	 * create them now, rather than waiting for the first build.
	 * This will present a more uniform interface to the developer.
	 */
	trace(("mark\n"));
	pconf_data = change_pconf_get(cp, 0);
	if
	(
		pconf_data->create_symlinks_before_build
	&&
		!pconf_data->remove_symlinks_after_build
	)
		change_create_symlinks_to_baseline(cp, up2);

	/*
	 * verbose success message
	 */
	trace(("mark\n"));
	change_verbose
	(
		cp,
		"ownership changed from user \"%S\" to user \"%S\"",
		user_name(up1),
		user_name(up2)
	);

	/*
	 * clean up and go home
	 */
	change_free(cp);
	project_free(pp);
	user_free(up);
	user_free(up1);
	user_free(up2);
	trace((/*{*/"}\n"));
}


void
change_owner()
{
	trace(("change_owner()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		change_owner_main();
		break;

	case arglex_token_help:
		change_owner_help();
		break;

	case arglex_token_list:
		change_owner_list();
		break;
	}
	trace((/*{*/"}\n"));
}
