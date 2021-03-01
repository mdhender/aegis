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
 * MANIFEST: functions for implementing integrate pass
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <aeip.h>
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


static void integrate_pass_usage _((void));

static void
integrate_pass_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf
	(
		stderr,
		"usage: %s -Integrate_PASS [ <option>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -Integrate_PASS -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -Integrate_PASS -Help\n", progname);
	quit(1);
}


static void integrate_pass_help _((void));

static void
integrate_pass_help()
{
	static char *text[] =
	{
#include <../man1/aeip.h>
	};

	help(text, SIZEOF(text), integrate_pass_usage);
}


static void integrate_pass_list _((void (*)(void)));

static void
integrate_pass_list(usage)
	void		(*usage)_((void));
{
	string_ty	*project_name;

	trace(("integrate_pass_list()\n{\n"/*}*/));
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


static void integrate_pass_main _((void));

static void
integrate_pass_main()
{
	time_t		youngest;
	string_ty	*hp;
	string_ty	*id;
	string_ty	*cwd;
	pstate		pstate_data;
	cstate		cstate_data;
	string_ty	*old_baseline;
	string_ty	*new_baseline;
	string_ty	*dev_dir;
	string_ty	*int_name;
	string_ty	*rev_name;
	string_ty	*dev_name;
	cstate_history	history_data;
	pstate_history	phistory_data;
	int		j;
	int		ncmds;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;
	user_ty		*devup;
	int		nerr;

	trace(("integrate_pass_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	nolog = 0;
	nerr = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(integrate_pass_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				integrate_pass_usage();
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				fatal("duplicate -Change option");
			change_number = arglex_value.alv_number;
			if (change_number < 1)
				fatal("change %ld out of range", change_number);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				integrate_pass_usage();
			/* fall through... */
	
		case arglex_token_string:
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (nolog)
			{
				fatal
				(
					"duplicate %s option",
					arglex_value.alv_string
				);
			}
			nolog = 1;
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
	 * Take an advisory write lock on the appropriate row of the change
	 * table.  Take an advisory write lock on the appropriate row of the
	 * user table.  Block until can get both simultaneously.
	 */
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	user_ustate_lock_prepare(up);
	project_build_write_lock_prepare(pp);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * It is an error if the change is not in the being_integrated state.
	 * It is an error if the change is not assigned to the current user.
	 * It is an error if the change has no current build.
	 * It is an error if the change has no current test pass.
	 * It is an error if the change has no current baseline test pass.
	 */
	if (cstate_data->state != cstate_state_being_integrated)
	{
		change_fatal
		(
			cp,
"this change is in the '%s' state, \
it must be in the 'being integrated' state to pass integration",
			cstate_state_ename(cstate_data->state)
		);
	}
	nerr = 0;
	youngest = 0;
	if (!str_equal(change_integrator_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
     "user \"%S\" is not the integrator, only user \"%S\" may pass integration",
			user_name(up),
			change_integrator_name(cp)
		);
	}
	if (!cstate_data->build_time)
	{
		change_error
		(
			cp,
"this change must successfully complete an '%s -Build'%s \
before it can pass integration",
			option_progname_get(),
			change_outstanding_builds(cp, youngest)
		);
		++nerr;
	}
	if (!cstate_data->test_exempt && !cstate_data->test_time)
	{
		change_error
		(
			cp,
"this change must successfully complete an '%s -Test'%s \
before it can pass integration",
			option_progname_get(),
			change_outstanding_tests(cp, youngest)
		);
		++nerr;
	}
	if
	(
		!cstate_data->test_baseline_exempt
	&&
		!cstate_data->test_baseline_time
	)
	{
		change_error
		(
			cp,
"this change must successfully complete an '%s -Test -BaseLine'%s \
before it can pass integration",
			option_progname_get(),
			change_outstanding_tests_baseline(cp, youngest)
		);
		++nerr;
	}
	if
	(
		!cstate_data->regression_test_exempt
	&&
		!cstate_data->regression_test_time
	)
	{
		change_error
		(
			cp,
"this change must successfully complete an '%s -Test -REGression'%s \
before it can pass integration",
			option_progname_get(),
			change_outstanding_tests_regression(cp, youngest)
		);
		++nerr;
	}
	if (nerr)
	{
		change_fatal
		(
			cp,
	     "found %d error%s, change remains in the 'being integrated' state",
			nerr,
			(nerr == 1 ? "" : "s")
		);
	}

	/*
	 * Make sure they aren't in a nuisance place.
	 * (Many systems can't delete a directory if any process has
	 * its current directory within it.)
	 */
	os_become_orig();
	cwd = os_curdir();
	os_become_undo();
	if (os_below_dir(change_development_directory_get(cp, 1), cwd))
		change_fatal(cp, "please leave development directory");
	id = change_integration_directory_get(cp, 1);
	if (os_below_dir(id, cwd))
		change_fatal(cp, "please leave integration directory");
	if (os_below_dir(project_baseline_path_get(pp, 1), cwd))
		project_fatal(pp, "please leave baseline directory");

	/*
	 * add to history for state change
	 */
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_integrate_pass;
	phistory_data = project_history_new(pp);
	phistory_data->delta_number = cstate_data->delta_number;
	phistory_data->change_number = change_number;

	/*
	 * note that the project has no current integration
	 */
	pstate_data->currently_integrating_change = 0;

	/*
	 * Clear the default-change field of the user row.
	 * Remove the change from the list of assigned changes in the user
	 * change table (in the user row).
	 */
	user_own_remove(up, project_name_get(pp), change_number);

	/*
	 * add all of the generated files in the project to this change
	 * so that their history is remembered
	 */
	for (j = 0; j < pstate_data->src->length; ++j)
	{
		cstate_src	c_src_data;
		pstate_src	p_src_data;

		p_src_data = pstate_data->src->list[j];
		if (p_src_data->usage != file_usage_build)
			continue;
		if (p_src_data->deleted_by)
			continue;
		if (p_src_data->about_to_be_created_by)
			continue;
		c_src_data = change_src_find(cp, p_src_data->file_name);
		if (c_src_data)
			continue;
		c_src_data = change_src_new(cp);
		c_src_data->file_name = str_copy(p_src_data->file_name);
		c_src_data->edit_number = str_copy(p_src_data->edit_number);
		c_src_data->action = file_action_modify;
		c_src_data->usage = p_src_data->usage;
	}

	/*
	 * Update the edit history of each changed file.
	 * Update the edit number of each file.
	 * Unlock each file.
	 */
	if (!nolog)
	{
		user_ty	*pup;

		pup = project_user(pp);
		log_open(change_logfile_get(cp), pup, log_style_create);
		user_free(pup);
	}
	ncmds = 0;
	hp = project_history_path_get(pp);
	trace(("mark\n"));
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		cstate_src	c_src_data;
		pstate_src	p_src_data;
		string_ty	*src_file;
		string_ty	*dst_file;

		c_src_data = cstate_data->src->list[j];
		c_src_data->diff_time = 0;
		c_src_data->diff_file_time = 0;

		p_src_data = project_src_find(pp, c_src_data->file_name);
		if (!p_src_data)
			project_fatal(pp, "corrupted src field");
		p_src_data->locked_by = 0;
		switch (c_src_data->action)
		{
		case file_action_create:
			/*
			 * because history is never thrown away,
			 * we could be reusing an existing history file
			 */
			if (p_src_data->edit_number)
			{
				assert(p_src_data->deleted_by);
				p_src_data->deleted_by = 0;
				p_src_data->about_to_be_created_by = 0;
				c_src_data->edit_number =
					str_copy(p_src_data->edit_number);
				goto reusing_an_old_file;
			}

			/*
			 * it exists, now
			 */
			trace(("create\n"));
			p_src_data->about_to_be_created_by = 0;

			/*
			 * create the history
			 */
			change_run_history_create_command
			(
				cp,
				c_src_data->file_name
			);
			p_src_data->edit_number =
				change_run_history_query_command
				(
					cp,
					c_src_data->file_name
				);
			c_src_data->edit_number =
				str_copy(p_src_data->edit_number);
			break;

		case file_action_modify:
			reusing_an_old_file:
			trace(("modify\n"));
			assert(str_equal(p_src_data->edit_number,
				c_src_data->edit_number));

			/*
			 * prepare for the history commands
			 */
			src_file =
				str_format("%S/%S", id, c_src_data->file_name);
			dst_file =
				str_format("%S/%S", hp, c_src_data->file_name);

			/*
			 * update the history
			 */
			change_run_history_put_command
			(
				cp,
				c_src_data->file_name
			);
			p_src_data->edit_number =
				change_run_history_query_command
				(
					cp,
					c_src_data->file_name
				);
			c_src_data->edit_number =
				str_copy(p_src_data->edit_number);
			break;

		case file_action_remove:
			/*
			 * don't need to do much for deletions.
			 * Note that we never throw the history file away.
			 */
			trace(("remove\n"));
			assert(str_equal(p_src_data->edit_number,
				c_src_data->edit_number));
			p_src_data->deleted_by = change_number;
			break;
		}
	}

	/*
	 * Advance the change to the 'completed' state.
	 * Clear the build-time field.
	 * Clear the test-time field.
	 * Clear the test-baseline-time field.
	 */
	cstate_data->state = cstate_state_completed;
	change_build_times_clear(cp);
	dev_dir = str_copy(change_development_directory_get(cp, 1));
	change_development_directory_clear(cp);
	new_baseline = str_copy(change_integration_directory_get(cp, 1));
	change_integration_directory_clear(cp);
	int_name = change_integrator_name(cp);
	rev_name = change_reviewer_name(cp);
	dev_name = change_developer_name(cp);

	/*
	 * we are going to keep the delta,
	 * and throw away the old baseline
	 */
	old_baseline =
		str_format
		(
			"%S.D%3.3ld",
			project_baseline_path_get(pp, 1),
			cstate_data->delta_number - 1
		);
	new_baseline = change_integration_directory_get(cp, 1);
	project_become(pp);
	commit_rename(project_baseline_path_get(pp, 1), old_baseline);
	commit_rename(new_baseline, project_baseline_path_get(pp, 1));
	commit_rmdir_tree_errok(old_baseline);
	project_become_undo();
	str_free(old_baseline);

	/*
	 * throw away the development directory
	 */
	change_verbose(cp, "discard old directories");
	devup = user_symbolic(pp, dev_name);
	user_become(devup);
	commit_rmdir_tree_errok(dev_dir);
	user_become_undo();
	str_free(dev_dir);
	user_free(devup);

	/*
	 * Write the change table row.
	 * Write the user table row.
	 * Release advisory locks.
	 */
	change_cstate_write(cp);
	user_ustate_write(up);
	project_pstate_write(pp);
	commit();
	lock_release();

	/*
	 * run the notify command
	 */
	change_run_integrate_pass_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, "integrate pass");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
integrate_pass()
{
	trace(("integrate_pass()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		integrate_pass_main();
		break;

	case arglex_token_help:
		integrate_pass_help();
		break;

	case arglex_token_list:
		integrate_pass_list(integrate_pass_usage);
		break;
	}
	trace((/*{*/"}\n"));
}
