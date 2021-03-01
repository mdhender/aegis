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
 * MANIFEST: functions to implement develop end
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <aede.h>
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


static void develop_end_usage _((void));

static void
develop_end_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -Develop_End [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Develop_End -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Develop_End -Help\n", progname);
	quit(1);
}


static void develop_end_help _((void));

static void
develop_end_help()
{
	static char *text[] =
	{
#include <../man1/aede.h>
	};

	help(text, SIZEOF(text), develop_end_usage);
}


static void develop_end_list _((void));

static void
develop_end_list()
{
	string_ty	*project_name;

	trace(("develop_end_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_end_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				develop_end_usage();
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
		1 << cstate_state_being_developed
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


static void de_func _((void *, dir_walk_message_ty, string_ty *, struct stat *));

static void
de_func(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	change_ty	*cp;
	int		uid;

	trace(("de_func(message = %d, path = \"%s\", st = %08lX)\n{\n"/*}*/,
		message, path->str_text, st));
	cp = (change_ty *)arg;
	switch (message)
	{
	case dir_walk_file:
	case dir_walk_dir_after:
		os_become_query(&uid, (int *)0, (int *)0);
		if (st->st_uid == uid)
		{
			os_chmod(path, st->st_mode & 07555);
			undo_chmod(path, st->st_mode);
		}
		repair_diff_time(cp, path);
		break;

	case dir_walk_special:
	case dir_walk_symlink:
	case dir_walk_dir_before:
		break;
	}
	trace((/*{*/"}\n"));
}


static void develop_end_main _((void));

static void
develop_end_main()
{
	string_ty	*dd;
	cstate		cstate_data;
	pstate		pstate_data;
	int		j;
	cstate_history	history_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	int		diff_whine;
	int		errs;
	time_t		youngest;
	string_ty	*youngest_name;
	int		config_seen;
	string_ty	*config_name;

	trace(("develop_end_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_end_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				develop_end_usage();
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
				develop_end_usage();
			/* fall through... */
		
		case arglex_token_string:
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
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
	trace(("mark\n"));
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	user_ustate_lock_prepare(up);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * It is an error if the change is not in the in-development state.
	 * It is an error if the change is not assigned to the current user.
	 * It is an error if the change has no current diff.
	 * It is an error if the change has no current build.
	 * It is an error if the change has no current test pass.
	 * It is an error if the change has no current baseline test pass.
	 * It is an error if the change has no new test associtaed with it.
	 */
	trace(("mark\n"));
	if (cstate_data->state != cstate_state_being_developed)
	{
		change_fatal
		(
			cp,
 "this change is in the '%s' state, you cannot end development from this state",
			cstate_state_ename(cstate_data->state)
		);
	}
	if (!str_equal(change_developer_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
"user \"%S\" is not the developer, only user \"%s\" may end development of this change",
			user_name(up),
			change_developer_name(cp)
		);
	}
	if (!cstate_data->src->length)
	{
		change_fatal
		(
			cp,
"this change has no files, \
you must add some files to this change before you may end development"
		);
	}
	errs = 0;
	diff_whine = 0;
	config_seen = 0;

	/*
	 * It is an error if any files in the change file table have been
	 * modified since the last build.
	 */
	trace(("mark\n"));
	dd = change_development_directory_get(cp, 1);
	user_become(up);
	youngest = 0;
	youngest_name = 0;
	config_name = str_from_c(THE_CONFIG_FILE);
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		cstate_src	c_src_data;
		pstate_src	p_src_data;
		string_ty	*path;
		string_ty	*path_d;
		long		when;
		long		when_d;
		int		file_required;
		int		diff_file_required;

		trace(("mark\n"));
		file_required = 1;
		diff_file_required = 1;
		c_src_data = cstate_data->src->list[j];
		if (c_src_data->action == file_action_remove)
			file_required = 0;
		if (c_src_data->usage == file_usage_build)
		{
			file_required = 0;
			diff_file_required = 0;
		}

		/*
		 * the config file in a change
		 * implies additional tests
		 */
		if (str_equal(c_src_data->file_name, config_name))
			config_seen++;

		/*
		 * make sure the file exists
		 */
		path = str_format("%S/%S", dd, c_src_data->file_name);
		if (os_exists(path))
			when = os_mtime(path);
		else
			when = 0;
		if (file_required && !when)
		{
			change_error
			(
				cp,
				"file \"%s\" not found",
				c_src_data->file_name->str_text
			);
			str_free(path);
			errs++;
			continue;
		}

		/*
		 * make sure the filename conforms to length limits
		 *
		 * Scenario: user copies "config", alters filename
		 * constraints, creates file, uncopies "config".
		 * Reviewer will not necessarily notice, especially when
		 * expecting aegis to notice for him.
		 */
		if (file_required)
		{
			string_ty	*e;

			user_become_undo();
			e = change_filename_check(cp, c_src_data->file_name, 0);
			user_become(up);
			if (e)
			{
				change_error(cp, "%S", e);
				++errs;
				str_free(e);
			}
		}

		/*
		 * get the difference time
		 */
		path_d = str_format("%S,D", path);
		str_free(path);
		if (os_exists(path_d))
			when_d = os_mtime(path_d);
		else
			when_d = 0;
		str_free(path_d);

		if (file_required && when > youngest)
		{
			youngest = when;
			youngest_name = c_src_data->file_name;
		}
		if
		(
			(
				file_required
			&&
				(
					when != c_src_data->diff_time
				||
					!c_src_data->diff_time
				)
			)
		||
			(
				diff_file_required
			&&
				(
					when_d != c_src_data->diff_file_time
				||
					!c_src_data->diff_file_time
				)
			)
		)
		{
			if (!diff_whine)
			{
				if
				(
					file_required
				&&
					when != c_src_data->diff_time
				&&
					c_src_data->diff_time
				)
				{
					change_error
					(
						cp,
"the \"%S\" file was modified after the last difference, \
this change must successfully complete another '%s -Diff' \
before it can end development",
						c_src_data->file_name,
						option_progname_get()
					);
				}
				else if
				(
					diff_file_required
				&&
					when_d != c_src_data->diff_file_time
				&&
					c_src_data->diff_file_time
				)
				{
					change_error
					(
						cp,
"the \"%S,D\" file was modified after the last difference, \
this change must successfully complete another '%s -Diff' \
before it can end development",
						c_src_data->file_name,
						option_progname_get()
					);
				}
				else
				{
					change_error
					(
						cp,
"this change must successfully complete an '%s -Diff' \
before it can end development",
						option_progname_get()
					);
				}
				diff_whine++;
				errs++;
			}
		}

		/*
		 * It is an error if any files in the change file table have
		 * different edit numbers to the baseline file table edit
		 * numbers.
		 */
		p_src_data = project_src_find(pp, c_src_data->file_name);
		if (c_src_data->action != file_action_create)
		{
			if (!p_src_data)
			{
				change_error
				(
					cp,
					"file \"%S\" no longer in baseline",
					c_src_data->file_name
				);
				errs++;
			}
			if
			(
				!str_equal
				(
					c_src_data->edit_number,
					p_src_data->edit_number
				)
			)
			{
				change_error
				(
					cp,
      "file \"%S\" in baseline has changed since last '%s -DIFFerence' command",
					c_src_data->file_name,
					option_progname_get()
				);
				errs++;
			}

			/*
			 * make sure we can lock the file
			 */
			if (p_src_data->locked_by)
			{
				change_error
				(
					cp,
					"file \"%S\" locked for change %d",
					c_src_data->file_name,
					p_src_data->locked_by
				);
				errs++;
			}
		}
		else
		{
			if (p_src_data)
			{
				if (!p_src_data->deleted_by)
				{
					change_error
					(
						cp,
						"file \"%S\" already exists",
						c_src_data->file_name
					);
					++errs;
				}
			}
			else
			{
				/*
				 * add a new entry to the pstate src list,
				 * and mark it as "about to be created".
				 */
				p_src_data =
					project_src_new
					(
						pp,
						c_src_data->file_name
					);
			}
			p_src_data->usage = c_src_data->usage;
			p_src_data->about_to_be_created_by = change_number;
		}
		p_src_data->locked_by = change_number;
	}
	str_free(config_name);
	user_become_undo();

	/*
	 * if the config file changes,
	 * make sure the project file names still conform
	 */
	if (config_seen)
	{
		for (j = 0; j < pstate_data->src->length; ++j)
		{
			pstate_src	p_src_data;
			string_ty	*e;
	
			trace(("mark\n"));
			p_src_data = pstate_data->src->list[j];
			if (p_src_data->deleted_by)
				continue;
			if (p_src_data->about_to_be_created_by)
				continue;
			if (change_src_find(cp, p_src_data->file_name))
				continue;
	
			e = change_filename_check(cp, p_src_data->file_name, 0);
			if (e)
			{
				change_error(cp, "project %S", e);
				++errs;
				str_free(e);
			}
		}
	}

	/*
	 * verify that the youngest file is older than the
	 * build, test, test -bl and test -reg times
	 */
	trace(("mark\n"));
	if (!cstate_data->build_time || youngest >= cstate_data->build_time)
	{
		if (youngest_name && cstate_data->build_time)
		{
			change_error
			(
				cp,
"the \"%S\" file was modified after the last build, \
this change must successfully complete another '%s -Build'%s \
before it can end development",
				youngest_name,
				option_progname_get(),
				change_outstanding_builds(cp, youngest)
			);
		}
		else
		{
			change_error
			(
				cp,
"this change must successfully complete an '%s -Build'%s \
before it can end development",
				option_progname_get(),
				change_outstanding_builds(cp, youngest)
			);
		}
		++errs;
	}
	trace(("mark\n"));
	if
	(
		!cstate_data->test_exempt
	&&
		(
			!cstate_data->test_time
		||
			youngest >= cstate_data->test_time
		)
	)
	{
		change_error
		(
			cp,
"this change must successfully complete an '%s -Test'%s \
before it can end development",
			option_progname_get(),
			change_outstanding_tests(cp, youngest)
		);
		++errs;
	}
	trace(("mark\n"));
	if
	(
		!cstate_data->test_baseline_exempt
	&&
		(
			!cstate_data->test_baseline_time
		||
			youngest >= cstate_data->test_baseline_time
		)
	)
	{
		change_error
		(
			cp,
"this change must successfully complete an '%s -Test -BaseLine'%s \
before it can end development",
			option_progname_get(),
			change_outstanding_tests_baseline(cp, youngest)
		);
		++errs;
	}
	trace(("mark\n"));
	if
	(
		!cstate_data->regression_test_exempt
	&&
		(
			!cstate_data->regression_test_time
		||
			youngest >= cstate_data->regression_test_time
		)
	)
	{
		change_error
		(
			cp,
"this change must successfully complete an '%s -Test -REGression'%s \
before it can end development",
			option_progname_get(),
			change_outstanding_tests_regression(cp, youngest)
		);
		++errs;
	}

	/*
	 * if there was any problem,
	 * stay in 'being developed' state.
	 */
	trace(("mark\n"));
	if (errs)
	{
		change_fatal
		(
			cp,
	      "found %d error%s, change remains in the 'being developed' state",
			errs,
			(errs == 1 ? "" : "s")
		);
	}

	/*
	 * Change all of the files in the development directory
	 * to be read-only, and record the new ctime.
	 */
	trace(("mark\n"));
	user_become(up);
	dir_walk(dd, de_func, cp);
	user_become_undo();

	/*
	 * add to history for state change
	 */
	trace(("mark\n"));
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_develop_end;

	/*
	 * Advance the change to the being-reviewed state.
	 * Clear the build-time field.
	 * Clear the test-time field.
	 * Clear the test-baseline-time field.
	 */
	trace(("mark\n"));
	cstate_data->state = cstate_state_being_reviewed;
	change_build_times_clear(cp);

	/*
	 * Remove the change from the list of assigned changes in the user
	 * change table (in the user row).
	 */
	trace(("mark\n"));
	user_own_remove(up, project_name_get(pp), change_number);

	/*
	 * Write the change table row.
	 * Write the user table row.
	 * Release advisory locks.
	 */
	trace(("mark\n"));
	change_cstate_write(cp);
	project_pstate_write(pp);
	user_ustate_write(up);
	commit();
	lock_release();

	/*
	 * run the notify command
	 */
	change_run_develop_end_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, "development completed");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
develop_end()
{
	trace(("develop_end()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		develop_end_main();
		break;

	case arglex_token_help:
		develop_end_help();
		break;

	case arglex_token_list:
		develop_end_list();
		break;
	}
	trace((/*{*/"}\n"));
}
