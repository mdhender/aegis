/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
"NAME",
"	%s -Develop_End - complete development of a change",
"",
"SYNOPSIS",
"	%s -Develop_End [ <option>... ]",
"	%s -Develop_End -List [ <option>... ]",
"	%s -Develop_End -Help",
"",
"DESCRIPTION",
"	The %s -Develop_End command is used to notify",
"	%s of the completion of the development of a change.",
"",
"	Successful execution of the command advances the change",
"	from the 'being_developed' state to the 'being_reviewed'",
"	state.",
"",
"	The ownership of files in the development directory is",
"	changed to the project owner and group, and the files",
"	changed to be read-only.  This prevents accidental",
"	alterations of the change's files between development and",
"	integration.",
"",
"	The change is no longer considered assigned to the",
"	current user.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Change <number>",
"		This option may be used to specify a particular",
"		change within a project.  When no -Change option is",
"		specified, the AEGIS_CHANGE environment variable is",
"		consulted.  If that does not exist, the user's",
"		$HOME/.aegisrc file is examined for a default change",
"		field (see aeuconf(5) for more information).  If",
"		that does not exist, when the user is only working",
"		on one change within a project, that is the default",
"		change number.  Otherwise, it is an error.",
"",
"	-Help",
"		This option may be used to obtain more",
"		information about how to use the %s program.",
"",
"	-List",
"		This option may be used to obtain a list of",
"		suitable subjects for this command.  The list may",
"		be more general than expected.",
"",
"	-Project <name>",
"		This option may be used to select the project of",
"		interest.  When no -Project option is specified, the",
"		AEGIS_PROJECT environment variable is consulted.  If",
"		that does not exist, the user's $HOME/.aegisrc file",
"		is examined for a default project field (see",
"		aeuconf(5) for more information).  If that does not",
"		exist, when the user is only working on changes",
"		within a single project, the project name defaults",
"		to that project.  Otherwise, it is an error.",
"",
"	-TERse",
"		This option may be used to cause listings to",
"		produce the bare minimum of information.  It is",
"		usually useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause %s to produce",
"		more output.  By default %s only produces",
"		output on errors.  When used with the -List",
"		option this option causes column headings to be",
"		added.",
"",
"	All options may be abbreviated; the abbreviation is",
"	documented as the upper case letters, all lower case",
"	letters and underscores (_) are optional.  You must use",
"	consecutive sequences of optional letters.",
"",
"	All options are case insensitive, you may type them in",
"	upper case or lower case or a combination of both, case",
"	is not important.",
"",
"	For example: the arguments \"-project, \"-PROJ\" and \"-p\"",
"	are all interpreted to mean the -Project option.  The",
"	argument \"-prj\" will not be understood, because",
"	consecutive optional characters were not supplied.",
"",
"	Options and other command line arguments may be mixed",
"	arbitrarily on the command line, after the function",
"	selectors.",
"",
"	The GNU long option names are understood.  Since all",
"	option names for aegis are long, this means ignoring the",
"	extra leading '-'.  The \"--option=value\" convention is",
"	also understood.",
"",
"RECOMMENDED ALIAS",
"	The recommended alias for this command is",
"	csh%%	alias aede '%s -de \\!* -v'",
"	sh$	aede(){%s -de $* -v}",
"",
"ERRORS",
"	It is an error if the change is not assigned to the",
"	current user.",
"	It is an error if The change is not in the",
"	'being_developed' state.",
"	It is an error if there has been no successful '%s",
"	-Build' command since a change file was last edited.",
"	It is an error if there has been no successful '%s",
"	-DIFFerence' command since a change file was last edited.",
"	It is an error if there has been no successful '%s",
"	-Test' command since a change file was last edited.",
"	It is an error if there has been no successful '%s",
"	-Test -BaseLine' command since a change file was last",
"	edited.",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any",
"	error.	The %s command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
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
	int		build_whine;
	int		test_whine;
	int		test_bl_whine;
	int		reg_test_whine;
	int		diff_whine;
	int		errs;

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
	if (cstate_data->state != cstate_state_being_developed)
		change_fatal(cp, "not in 'being_developed' state");
	if (!str_equal(change_developer_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
			"user \"%S\" is not the developer",
			user_name(up)
		);
	}
	if (!cstate_data->src->length)
		change_fatal(cp, "no files");
	errs = 0;
	build_whine = 0;
	test_whine = 0;
	test_bl_whine = 0;
	reg_test_whine = 0;
	diff_whine = 0;

	/*
	 * It is an error if any files in the change file table have been
	 * modified since the last build.
	 * It is an error if any files in the change file table have been
	 * modified since the last diff.
	 */
	dd = change_development_directory_get(cp, 1);
	user_become(up);
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
		 * get the difference time
		 */
		path_d = str_format("%S,D", path);
		str_free(path);
		if (os_exists(path_d))
			when_d = os_mtime(path_d);
		else
			when_d = 0;
		str_free(path_d);

		if
		(
			file_required
		&&
			(
				when >= cstate_data->build_time
			||
				!cstate_data->build_time
			)
		)
		{
			if (!build_whine)
			change_error
			(
				cp,
				"no current '%s -Build' registration",
				option_progname_get()
			);
			build_whine++;
			errs++;
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
			change_error
			(
				cp,
				"no current '%s -Diff' registration",
				option_progname_get()
			);
			diff_whine++;
			errs++;
		}
		if
		(
			!cstate_data->test_exempt
		&&
			file_required
		&&
			(
				when >= cstate_data->test_time
			||
				!cstate_data->test_time
			)
		)
		{
			if (!test_whine)
			change_error
			(
				cp,
				"no current '%s -Test' registration",
				option_progname_get()
			);
			test_whine++;
			errs++;
		}
		if
		(
			!cstate_data->test_baseline_exempt
		&&
			file_required
		&&
			(
				when >= cstate_data->test_baseline_time
			||
				!cstate_data->test_baseline_time
			)
		)
		{
			if (!test_bl_whine)
			change_error
			(
				cp,
				"no current '%s -Test -BaseLine' registration",
				option_progname_get()
			);
			test_bl_whine++;
			errs++;
		}
		if
		(
			!cstate_data->regression_test_exempt
		&&
			file_required
		&&
			(
				when >= cstate_data->regression_test_time
			||
				!cstate_data->regression_test_time
			)
		)
		{
			if (!reg_test_whine)
			change_error
			(
				cp,
			       "no current '%s -Test -REGression' registration",
				option_progname_get()
			);
			reg_test_whine++;
			errs++;
		}

		/*
		 * It is an error if any files in the change file table haved
		 * different edit numbers to the baseline file table edit
		 * numbers.
		 */
		if (c_src_data->action != file_action_create)
		{
			p_src_data = project_src_find(pp, c_src_data->file_name);
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
			/*
			 * add a new entry to the pstate src list,
			 * and mark it as "about to be created".
			 */
			p_src_data = project_src_new(pp, c_src_data->file_name);
			p_src_data->usage = c_src_data->usage;
			p_src_data->about_to_be_created_by = change_number;
		}
		p_src_data->locked_by = change_number;
	}
	if (errs)
		quit(1);

	/*
	 * Change all of the files in the development directory
	 * to be read-only, and record the new ctime.
	 */
	dir_walk(dd, de_func, cp);
	user_become_undo();

	/*
	 * add to history for state change
	 */
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_develop_end;

	/*
	 * Advance the change to the being-reviewed state.
	 * Clear the build-time field.
	 * Clear the test-time field.
	 * Clear the test-baseline-time field.
	 */
	cstate_data->state = cstate_state_being_reviewed;
	cstate_data->build_time = 0;
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;
	cstate_data->regression_test_time = 0;

	/*
	 * Remove the change from the list of assigned changes in the user
	 * change table (in the user row).
	 */
	user_own_remove(up, project_name_get(pp), change_number);

	/*
	 * Write the change table row.
	 * Write the user table row.
	 * Release advisory locks.
	 */
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
