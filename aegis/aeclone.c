/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to implement the 'aegis -CLone' command
 */

#include <ac/stdio.h>

#include <aeclone.h>
#include <ael/change/by_state.h>
#include <aenc.h>
#include <arglex2.h>
#include <change.h>
#include <change_bran.h>
#include <change/file.h>
#include <commit.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <project_hist.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void clone_usage _((void));

static void
clone_usage()
{
	char	*progname;

	progname = progname_get();
	fprintf(stderr, "usage: %s -CLone [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -CLone -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -CLone -Help\n", progname);
	quit(1);
}


static void clone_help _((void));

static void
clone_help()
{
	help("aeclone", clone_usage);
}


static void clone_list _((void));

static void
clone_list()
{
	string_ty	*project_name;

	trace(("clone_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(clone_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, clone_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, clone_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_changes_in_state_mask
	(
		project_name,
		~(1 << cstate_state_awaiting_development)
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static long get_change_number _((void));

static long
get_change_number()
{
	long		cn;

	cn = arglex_value.alv_number;
	if (cn == 0)
		cn = MAGIC_ZERO;
	else if (cn < 1)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Number", "%ld", cn);
		fatal_intl(scp, i18n("change $number out of range"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	return cn;
}


static void clone_main _((void));

static void
clone_main()
{
	sub_context_ty	*scp;
	string_ty	*project_name;
	long		change_number;
	long		change_number2;
	project_ty	*pp;
	project_ty	*pp2;
	user_ty		*up;
	change_ty	*cp;
	change_ty	*cp2;
	cstate		cstate_data;
	cstate		cstate_data2;
	cstate_history	history_data;
	string_ty	*usr;
	string_ty	*devdir;
	size_t		j;
	pconf		pconf_data;
	char		*branch;
	int		trunk;
	int		grandparent;
	int		mode;
	char		*output;

	trace(("clone_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	change_number2 = 0;
	usr = 0;
	devdir = 0;
	branch = 0;
	trunk = 0;
	grandparent = 0;
	output = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(clone_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, clone_usage);
			/* fall through... */

		case arglex_token_number:
			if (!change_number2)
			{
				/* change to clone */
				change_number2 = get_change_number();
			}
			else if (!change_number)
			{
				/* change to create */
				change_number = get_change_number();
			}
			else
				duplicate_option_by_name(arglex_token_change, clone_usage);
			break;

		case arglex_token_project:
			if (project_name)
				duplicate_option(clone_usage);
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, clone_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_directory:
			if (devdir)
				duplicate_option(clone_usage);
			if (arglex() != arglex_token_string)
				option_needs_dir(arglex_token_directory, clone_usage);
			devdir = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_branch:
			if (branch)
				duplicate_option(clone_usage);
			switch (arglex())
			{
			default:
				option_needs_number(arglex_token_branch, clone_usage);

			case arglex_token_number:
			case arglex_token_string:
				branch = arglex_value.alv_string;
				break;

			case arglex_token_stdio:
				branch = "";
				break;
			}
			break;

		case arglex_token_trunk:
			if (trunk)
				duplicate_option(clone_usage);
			++trunk;
			break;

		case arglex_token_grandparent:
			if (grandparent)
				duplicate_option(clone_usage);
			++grandparent;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(clone_usage);
			break;

		case arglex_token_whiteout:
		case arglex_token_whiteout_not:
			user_whiteout_argument(clone_usage);
			break;

		case arglex_token_output:
			if (output)
				duplicate_option(clone_usage);
			switch (arglex())
			{
			default:
				option_needs_file(arglex_token_output, clone_usage);
				/* NOTREACHED */

			case arglex_token_string:
				output = arglex_value.alv_string;
				break;

			case arglex_token_stdio:
				output = "";
				break;
			}
			break;
		}
		arglex();
	}
	if (!change_number2)
		fatal_intl(0, i18n("no change number"));
	if (change_number && output)
	{
		mutually_exclusive_options
		(
			arglex_token_change,
			arglex_token_output,
			clone_usage
		);
	}
	if (grandparent)
	{
		if (branch)
		{
			mutually_exclusive_options
			(
				arglex_token_branch,
				arglex_token_grandparent,
				clone_usage
			);
		}
		if (trunk)
		{
			mutually_exclusive_options
			(
				arglex_token_trunk,
				arglex_token_grandparent,
				clone_usage
			);
		}
		branch = "..";
	}
	if (trunk)
	{
		if (branch)
		{
			mutually_exclusive_options
			(
				arglex_token_branch,
				arglex_token_trunk,
				clone_usage
			);
		}
		branch = "";
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
	 * make sure this branch of the project is still active
	 */
	if (!change_is_a_branch(project_change_get(pp)))
		project_fatal(pp, 0, i18n("branch completed"));

	/*
	 * locate user data
	 */
	up = user_executing(pp);

	/*
	 * Lock the project state file.
	 * Block if necessary.
	 */
	project_pstate_lock_prepare(pp);
	user_ustate_lock_prepare(up);
	lock_take();

	/*
	 * make sure they are allowed to
	 */
	new_change_check_permission(pp, up);

	/*
	 * locate which branch
	 */
	if (branch)
		pp2 = project_find_branch(pp, branch);
	else
		pp2 = pp;

	/*
	 * locate change data
	 *	on the other branch
	 */
	assert(change_number2);
	cp2 = change_alloc(pp2, change_number2);
	change_bind_existing(cp2);
	cstate_data2 = change_cstate_get(cp2);

	/*
	 * It is an error if the other change is not at or after the
	 * 'being developed' state
	 */
	if (cstate_data2->state < cstate_state_being_developed)
		change_fatal(cp2, 0, i18n("bad clone state"));

	/*
	 * create a new change
	 */
	if (!change_number)
		change_number = project_next_change_number(pp, 1);
	else
	{
		if (project_change_number_in_use(pp, change_number))
		{
			scp = sub_context_new();
			sub_var_set(scp, "Number", "%ld", magic_zero_decode(change_number));
			project_fatal(pp, scp, i18n("change $number used"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
	}
	cp = change_alloc(pp, change_number);
	change_bind_new(cp);
	cstate_data = change_cstate_get(cp);

	/*
	 * copy change attributes from the old change
	 */
	assert(cstate_data2->description);
	if (cstate_data2->description)
		cstate_data->description = str_copy(cstate_data2->description);
	assert(cstate_data2->brief_description);
	cstate_data->brief_description =
		str_copy(cstate_data2->brief_description);
	cstate_data->cause = cstate_data2->cause;
	cstate_data->test_exempt = cstate_data2->test_exempt;
	cstate_data->test_baseline_exempt =
		cstate_data2->test_baseline_exempt;
	cstate_data->regression_test_exempt =
		cstate_data2->regression_test_exempt;
	if (cstate_data2->architecture)
	{
		change_architecture_clear(cp);
		for (j = 0; j < cstate_data2->architecture->length; ++j)
			change_architecture_add(cp, cstate_data2->architecture->list[j]);
	}
	change_copyright_years_now(cp);
	change_copyright_years_merge(cp, cp2);

	/*
	 * add to history for change creation
	 */
	cstate_data->state = cstate_state_awaiting_development;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_new_change;

	/*
	 * Construct the name of the development directory.
	 *
	 * (Do this before the state advances to being developed,
	 * it tries to find the config file in the as-yet
	 * non-existant development directory.)
	 */
	if (!devdir)
	{
		scp = sub_context_new();
		devdir = change_development_directory_template(cp, up);
		sub_var_set(scp, "File_Name", "%S", devdir);
		change_verbose(cp, scp, i18n("development directory \"$filename\""));
		sub_context_delete(scp);
	}
	change_development_directory_set(cp, devdir);

	/*
	 * add to history for develop begin
	 */
	cstate_data->state = cstate_state_being_developed;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_develop_begin;

	/*
	 * Clear the build-time field.
	 * Clear the test-time field.
	 * Clear the test-baseline-time field.
	 * Clear the src field.
	 */
	change_build_times_clear(cp);

	/*
	 * Assign the new change to the user.
	 */
	user_own_add(up, project_name_get(pp), change_number);

	/*
	 * Create the development directory.
	 */
	user_become(up);
	os_mkdir(devdir, 02755);
	undo_rmdir_bg(devdir);
	user_become_undo();

	/*
	 * Determine the correct mode for the copied files.
	 */
	mode = 0644 & ~change_umask(cp);

	/*
	 * add all of the files to the new change
	 * copy the files into the development directory
	 */
	change_verbose(cp, 0, i18n("copy change source files"));
	for (j = 0; ; ++j)
	{
		string_ty	*from;
		string_ty	*to;
		fstate_src	src_data;
		fstate_src	src_data2;
		fstate_src	p_src_data;

		/*
		 * find the file
		 *
		 * There are many files we will ignore.
		 */
		src_data2 = change_file_nth(cp2, j);
		if (!src_data2)
			break;
		if (src_data2->action == file_action_insulate)
			continue;
		if
		(
			src_data2->usage == file_usage_build
		&&
			src_data2->action != file_action_create
		&&
			src_data2->action != file_action_remove
		)
			continue;

		/*
		 * find the file in the project
		 */
		p_src_data = project_file_find(pp, src_data2->file_name);
		if (p_src_data)
		{
			if
			(
				p_src_data->deleted_by
			||
				p_src_data->about_to_be_created_by
			)
				p_src_data = 0;
		}
		if (!p_src_data)
		{
			if (src_data2->action == file_action_remove)
				continue;
			src_data2->action = file_action_create;
		}

		/*
		 * create the file in the new change
		 */
		src_data = change_file_new(cp, src_data2->file_name);
		src_data->action = src_data2->action;
		src_data->usage = src_data2->usage;

		/*
		 * removed files aren't copied,
		 * they have whiteout instead.
		 */
		if (src_data->action == file_action_remove)
		{
			change_file_whiteout_write(cp, src_data->file_name, up);
			continue;
		}

		/*
		 * If the change has already been completed, get the
		 * file from history, but if it is still active, get
		 * the file from the old development directory.
		 */
		if (cstate_data2->state == cstate_state_completed)
		{
			/*
			 * We could be creating the file, from the point
			 * of view of this branch.
			 */
			assert(src_data2->edit_number);
			if (p_src_data && src_data2->edit_number)
			{
				src_data->edit_number_origin =
					str_copy(src_data2->edit_number);
			}
			else if (p_src_data && p_src_data->edit_number)
			{
				src_data->edit_number_origin =
					str_copy(p_src_data->edit_number);
			}
			else
				src_data->action = file_action_create;

			/*
			 * figure where to send it
			 */
			to = str_format("%S/%S", devdir, src_data2->file_name);

			/*
			 * make sure there is a directory for it
			 */
			user_become(up);
			os_mkdir_between(devdir, src_data2->file_name, 02755);
			if (os_exists(to))
				os_unlink(to);
			os_unlink_errok(to);
			user_become_undo();

			/* 
			 * get the file from history
			 */
			change_run_history_get_command
			(
				cp2,
				src_data2->file_name,
				src_data2->edit_number,
				to,
				up
			);

			/*
			 * set the file mode
			 */
			user_become(up);
			os_chmod(to, mode);
			user_become_undo();

			/*
			 * clean up afterwards
			 */
			str_free(to);
		}
		else
		{
			/*
			 * If possible, use the edit number origin of
			 * the change we are cloning, this gives us the
			 * best chance to merge correctly.
			 *
			 * Otherwise, see if the file exists in the
			 * project and copy the head revision number
			 */
			if (p_src_data && src_data2->edit_number_origin)
			{
				src_data->edit_number_origin =
					str_copy(src_data2->edit_number_origin);
			}
			else if (p_src_data && p_src_data->edit_number)
			{
				src_data->edit_number_origin =
					str_copy(p_src_data->edit_number);
			}
			else
				src_data->action = file_action_create;

			/*
			 * construct the paths to the files
			 */
			from = change_file_path(cp2, src_data2->file_name);
			to = str_format("%S/%S", devdir, src_data2->file_name);

			/*
			 * copy the file
			 */
			user_become(up);
			os_mkdir_between(devdir, src_data2->file_name, 02755);
			if (os_exists(to))
				os_unlink(to);
			copy_whole_file(from, to, 0);

			/*
			 * set the file mode
			 */
			os_chmod(to, mode);
			user_become_undo();

			/* 
			 * clean up afterwards
			 */
			str_free(from);
			str_free(to);
		}
	}

	/*
	 * Write out the change file.
	 * There is no need to lock this file
	 * as it does not exist yet;
	 * the project state file, with the number in it, is locked.
	 */
	change_cstate_write(cp);

	/*
	 * Add the change to the list of existing changes.
	 * Increment the next_change_number.
	 * and write pstate back out.
	 */
	project_change_append(pp, change_number, 0);

	/*
	 * If there is an output option,
	 * write the change number to the file.
	 */
	if (output)
	{
		string_ty	*content;

		content = str_format("%ld", magic_zero_decode(change_number));
		if (*output)
		{
			string_ty	*fn;
			
			user_become(up);
			fn = str_from_c(output);
			file_from_string(fn, content, 0644);
			str_free(fn);
			user_become_undo();
		}
		else
			cat_string_to_stdout(content);
		str_free(content);
	}

	/*
	 * Write the change table row.
	 * Write the user table rows.
	 * Release advisory locks.
	 */
	project_pstate_write(pp);
	user_ustate_write(up);
	commit();
	lock_release();

	/*
	 * run the develop begin command
	 */
	change_run_develop_begin_command(cp, up);

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
	scp = sub_context_new();
	sub_var_set(scp, "ORiginal", "%S", change_version_get(cp2));
	sub_var_optional(scp, "ORiginal");
	change_verbose(cp, scp, i18n("clone complete"));
	sub_context_delete(scp);

	/*
	 * clean up and go home
	 */
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
clone()
{
	trace(("clone()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		clone_main();
		break;

	case arglex_token_help:
		clone_help();
		break;

	case arglex_token_list:
		clone_list();
		break;
	}
	trace((/*{*/"}\n"));
}
