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
 * MANIFEST: functions for implementing integrate fail
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <aeif.h>
#include <ael.h>
#include <arglex2.h>
#include <commit.h>
#include <change.h>
#include <dir.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void integrate_fail_usage _((void));

static void
integrate_fail_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -Integrate_FAIL -File <reason-file> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_FAIL -Edit [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_FAIL -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_FAIL -Help\n", progname);
	quit(1);
}


static void integrate_fail_help _((void));

static void
integrate_fail_help()
{
	static char *text[] =
	{
#include <../man1/aeif.h>
	};

	help(text, SIZEOF(text), integrate_fail_usage);
}


static void integrate_fail_list _((void (*)(void)));

static void
integrate_fail_list(usage)
	void		(*usage)_((void));
{
	string_ty	*project_name;

	trace(("integrate_fail_list()\n{\n"/*}*/));
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
	list_changes_in_state_mask
	(
		project_name,
		1 << cstate_state_being_integrated
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void repair_diff_time _((change_ty *, string_ty *));

static void
repair_diff_time(cp, path)
	change_ty	*cp;
	string_ty	*path;
{
	string_ty	*s;
	cstate_src	src_data;
	string_ty	*s2;

	s = os_below_dir(change_development_directory_get(cp, 1), path);
	src_data = change_src_find(cp, s);
	if (src_data)
		src_data->diff_time = os_mtime(path);
	else
	{
		if
		(
			s->str_length > 2
		&&
			!strcmp(s->str_text + s->str_length - 2, ",D")
		)
		{
			s2 = str_n_from_c(s->str_text, s->str_length - 2);
			src_data = change_src_find(cp, s2);
			if (src_data)
				src_data->diff_file_time = os_mtime(path);
			str_free(s2);
		}
	}
	str_free(s);
}


static void if_func _((void *, dir_walk_message_ty, string_ty *, struct stat *));

static void
if_func(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	change_ty	*cp;
	int		uid;

	trace(("if_func(message = %d, path = \"%s\", st = %08lX)\n{\n"/*}*/,
		message, path->str_text, st));
	cp = (change_ty *)arg;
	switch (message)
	{
	case dir_walk_dir_before:
	case dir_walk_file:
		os_become_query(&uid, (int *)0, (int *)0);
		if (st->st_uid == uid)
		{
			os_chmod
			(
				path,
				(st->st_mode | 0644) & ~change_umask(cp)
			);
			undo_chmod(path, st->st_mode);
			repair_diff_time(cp, path);
		}
		break;

	case dir_walk_special:
	case dir_walk_symlink:
	case dir_walk_dir_after:
		break;
	}
	trace((/*{*/"}\n"));
}


static void check_permissions _((change_ty *, user_ty *));

static void
check_permissions(cp, up)
	change_ty	*cp;
	user_ty		*up;
{
	cstate		cstate_data;

	cstate_data = change_cstate_get(cp);

	/*
	 * it is an error if the change is not in the 'being_integrated' state.
	 */
	if (cstate_data->state != cstate_state_being_integrated)
	{
		change_fatal
		(
			cp,
"this change is in the '%s' state, \
it must be in the 'being integrated' state to fail integration",
			cstate_state_ename(cstate_data->state)
		);
	}
	if (!str_equal(change_integrator_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
	"user \"%S\" not the integrator, only user \"%S\" may fail integration",
			user_name(up),
			change_integrator_name(cp)
		);
	}
}


static void integrate_fail_main _((void));

static void
integrate_fail_main()
{
	string_ty	*dd;
	cstate		cstate_data;
	pstate		pstate_data;
	cstate_history	history_data;
	string_ty	*comment = 0;
	string_ty	*rev_name;
	string_ty	*int_name;
	string_ty	*dir;
	int		j;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	user_ty		*devup;
	int		edit;

	trace(("integrate_fail_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	edit = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(integrate_fail_usage);
			continue;

		case arglex_token_string:
			error
			(
"warning: please use the -File option when specifying a reason file, \
the unadorned form is now obsolescent"
			);
			if (comment)
				fatal("too many files named");
			goto read_reason_file;

		case arglex_token_file:
			if (comment)
				goto duplicate;
			if (arglex() != arglex_token_string)
			{
				error
				(
				 "the -File option requires a filename argument"
				);
				integrate_fail_usage();
			}
			read_reason_file:
			os_become_orig();
			comment = read_whole_file(arglex_value.alv_string);
			os_become_undo();
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				integrate_fail_usage();
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
				goto duplicate;
			if (arglex() != arglex_token_string)
				integrate_fail_usage();
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_edit:
			if (edit)
			{
				duplicate:
				fatal
				(
					"duplicate %s option",
					arglex_value.alv_string
				);
			}
			++edit;
			break;
		}
		arglex();
	}
	if (edit && comment)
		fatal("may not use both the -File and -Edit options");
	if (!edit && !comment)
	{
		error("warning: no -File specified, assuming -Edit desired");
		++edit;
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
	 * locate change data
	 */
	if (!change_number)
		change_number = user_default_change(up);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	/*
	 * create the comments, if required
	 */
	if (edit)
	{
		/*
		 * make sure they are allowed to first,
		 * to avoid a wasted edit
		 */
		check_permissions(cp, up);
		comment = os_edit_new();
	}

	/*
	 * lock the change for writing
	 */
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	lock_prepare_ustate_all(); /* we don't know which users until later */
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * make sure they are allowed to
	 * (even if edited, could have changed during edit)
	 */
	check_permissions(cp, up);

	/*
	 * Change the state.
	 * Add to the change's history.
	 */
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_integrate_fail;
	history_data->why = comment;
	change_build_times_clear(cp);
	rev_name = change_reviewer_name(cp);
	int_name = change_integrator_name(cp);
	cstate_data->delta_number = 0;
	dir = str_copy(change_integration_directory_get(cp, 1));
	change_integration_directory_clear(cp);
	cstate_data->state = cstate_state_being_developed;

	/*
	 * Complain if they are in the integration directory,
	 * because the rmdir at the end can't then run to completion.
	 */
	os_become_orig();
	if (os_below_dir(dir, os_curdir()))
		change_fatal(cp, "please leave the integration directory");
	os_become_undo();

	/*
	 * note that the project has no current integration
	 */
	pstate_data->currently_integrating_change = 0;

	/*
	 * Remove it from the integrator's change list, and
	 * add it back into the developer's change list.
	 */
	user_own_remove(up, project_name_get(pp), change_number);
	devup = user_symbolic(pp, change_developer_name(cp));
	user_own_add(devup, project_name_get(pp), change_number);

	/*
	 * make the change files writable again
	 */
	change_verbose(cp, "make the development directory writable again");
	dd = change_development_directory_get(cp, 1);
	user_become(devup);
	dir_walk(dd, if_func, cp);
	user_become_undo();

	/*
	 * go through the files in the change and unlock them
	 * in the baseline
	 */
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		cstate_src	c_src_data;
		pstate_src	p_src_data;

		c_src_data = cstate_data->src->list[j];
		p_src_data = project_src_find(pp, c_src_data->file_name);
		if (!p_src_data)
			continue;
		p_src_data->locked_by = 0;

		/*
		 * Remove the file if it is about_to_be_created
		 * by the change we are rescinding.
		 */
		if (p_src_data->about_to_be_created_by)
		{
			assert(p_src_data->about_to_be_created_by == change_number);
			if (p_src_data->deleted_by)
				p_src_data->about_to_be_created_by = 0;
			else
				project_src_remove(pp, c_src_data->file_name);
		}
	}

	/*
	 * remove the integration directory
	 */
	change_verbose(cp, "remove the integration directory");
	project_become(pp);
	commit_rmdir_tree_errok(dir);
	project_become_undo();

	/*
	 * write out the data and release the locks
	 */
	change_cstate_write(cp);
	user_ustate_write(up);
	user_ustate_write(devup);
	project_pstate_write(pp);
	str_free(dir);
	commit();
	lock_release();

	/*
	 * run the notify command
	 */
	change_run_integrate_fail_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, "failed integration, returned to developer");
	change_free(cp);
	project_free(pp);
	user_free(up);
	user_free(devup);
	trace((/*{*/"}\n"));
}


void
integrate_fail()
{
	trace(("integrate_fail()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		integrate_fail_main();
		break;

	case arglex_token_help:
		integrate_fail_help();
		break;

	case arglex_token_list:
		integrate_fail_list(integrate_fail_usage);
		break;
	}
	trace((/*{*/"}\n"));
}
