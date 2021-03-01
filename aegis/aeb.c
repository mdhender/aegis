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
 * MANIFEST: functions to perform development and integration builds
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/time.h>
#include <errno.h>

#include <aeb.h>
#include <ael.h>
#include <arglex2.h>
#include <col.h>
#include <commit.h>
#include <change.h>
#include <error.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <word.h>


/*
 * NAME
 *	build_usage
 *
 * SYNOPSIS
 *	void build_usage(void);
 *
 * DESCRIPTION
 *	The build_usage function is used to
 *	briefly describe how to used the 'aegis -Build' command.
 */

static void build_usage _((void));

static void
build_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -Build [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Build -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Build -Help\n", progname);
	quit(1);
}


/*
 * NAME
 *	build_help
 *
 * SYNOPSIS
 *	void build_help(void);
 *
 * DESCRIPTION
 *	The build_help function is used to
 *	describe in detail how to use the 'aegis -Build' command.
 */

static void build_help _((void));

static void
build_help()
{
	static char *text[] =
	{
#include <../man1/aeb.h>
	};

	help(text, SIZEOF(text), build_usage);
}


/*
 * NAME
 *	build_list
 *
 * SYNOPSIS
 *	void build_list(void);
 *
 * DESCRIPTION
 *	The build_list function is used to
 *	list the changes which may be built within the project.
 */

static void build_list _((void));

static void
build_list()
{
	string_ty	*project_name;

	trace(("build_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(build_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				build_usage();
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
		(
			(1 << cstate_state_being_developed)
		|
			(1 << cstate_state_being_integrated)
		)
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	build_main
 *
 * SYNOPSIS
 *	void build_main(void);
 *
 * DESCRIPTION
 *	The build_main function is used to build a change in the "being
 *	developed" or "being integrated" states.  It extracts what to
 *	do from the command line.
 */

static void build_main _((void));

static void
build_main()
{
	cstate		cstate_data;
	pstate		pstate_data;
	pconf		pconf_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;
	wlist		partial;
	string_ty	*s1;
	string_ty	*s2;

	trace(("build_main()\n{\n"/*}*/));
	nolog = 0;
	project_name = 0;
	change_number = 0;
	wl_zero(&partial);
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(build_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				build_usage();
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
				build_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_file:
			if (arglex() != arglex_token_string)
			{
				error
				(
		   "The -File option must be followed by one or more file names"
				);
				build_usage();
			}
			/* fall through... */

		case arglex_token_string:
			s1 = str_from_c(arglex_value.alv_string);
			os_become_orig();
			s2 = os_pathname(s1, 1);
			os_become_undo();
			if (wl_member(&partial, s2))
				fatal("file \"%S\" named more than once", s1);
			wl_append(&partial, s2);
			str_free(s1);
			str_free(s2);
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
	 * Take an advisory write lock on this row of the change table.
	 * Block if necessary.
	 */
	trace(("mark\n"));
	if (!partial.wl_nwords)
	{
		change_cstate_lock_prepare(cp);
		project_build_read_lock_prepare(pp);
		lock_take();
	}
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	if (partial.wl_nwords)
	{
		string_ty	*dd;
		string_ty	*bl;
		size_t		j;

		/*
		 * resolve the path of each file
		 * 1.	the absolute path of the file name is obtained
		 * 2.	if the file is inside the development directory, ok
		 * 3.	if the file is inside the baseline, ok
		 * 4.	if neither, error
		 */
		dd = change_development_directory_get(cp, 1);
		bl = project_baseline_path_get(pp, 1);
		for (j = 0; j < partial.wl_nwords; ++j)
		{
			s1 = partial.wl_word[j];
			assert(s1->str_text[0] == '/');
			s2 = os_below_dir(dd, s1);
			if (!s2)
				s2 = os_below_dir(bl, s1);
			if (!s2)
				change_fatal(cp, "path \"%S\" unrelated", s1);
			str_free(s1);
			partial.wl_word[j] = s2;
		}
	}

	/*
	 * Extract the appropriate row of the change table.
	 * It is an error if the change is not in the in-development state.
	 * It is an error if the change is not assigned to the current user.
	 * It is an error if the change has no files assigned.
	 */
	trace(("mark\n"));
	switch (cstate_data->state)
	{
	default:
		change_fatal
		(
			cp,
"this change is in the '%s' state, \
it must be in the 'being developed' state to do a build",
			cstate_state_ename(cstate_data->state)
		);
		break;

	case cstate_state_being_developed:
		if (!str_equal(change_developer_name(cp), user_name(up)))
		{
			change_fatal
			(
				cp,
	    "user \"%S\" is not the developer, only user \"%S\" may do a build",
				user_name(up),
				change_developer_name(cp)
			);
		}
		assert(cstate_data->src);
		if (!cstate_data->src->length)
		{
			change_fatal
			(
				cp,
   "this change has no files, you must add some files before you may do a build"
			);
		}
		break;

	case cstate_state_being_integrated:
		if (!str_equal(change_integrator_name(cp), user_name(up)))
		{
			change_fatal
			(
				cp,
	   "user \"%S\" is not the integrator, only user \"%S\" may do a build",
				user_name(up),
				change_integrator_name(cp)
			);
		}
		if (partial.wl_nwords)
		{
			change_fatal
			(
				cp,
	      "you may not do a partial build in the \"being integrated\" state"
			);
		}
		break;
	}

	/*
	 * Update the time the build was done.
	 * This will not be written out if the build fails.
	 */
	os_throttle();
	trace(("mark\n"));
	change_build_time_set(cp);

	/*
	 * get the command to execute
	 *  1. if the change is editing config, use that
	 *  2. if the baseline contains config, use that
	 *  3. error if can't find one (DON'T look for file existence)
	 */
	trace(("mark\n"));
	pconf_data = change_pconf_get(cp, 1);

	/*
	 * the program has changed, so it needs testing again,
	 * so stomp on the validation fields.
	 */
	trace(("nuke time stamps\n"));
	change_test_times_clear(cp);

	/*
	 * do the build
	 */
	trace(("open the log file\n"));
	trace(("do the build\n"));
	if (cstate_data->state == cstate_state_being_integrated)
	{
		if (!nolog)
		{
			user_ty		*pup;

			pup = project_user(pp);
			log_open
			(
				change_logfile_get(cp),
				pup,
				log_style_snuggle
			);
			user_free(pup);
		}
		change_verbose(cp, "integration build started");
		change_run_build_command(cp);
		change_verbose(cp, "integration build complete");
	}
	else
	{
		if (!nolog)
			log_open(change_logfile_get(cp), up, log_style_snuggle);
		if (pconf_data->create_symlinks_before_build)
			change_create_symlinks_to_baseline(cp, up);
		change_verbose
		(
			cp,
			"%s build started",
			(partial.wl_nwords ? "partial" : "development")
		);
		change_run_project_file_command(cp);
		change_run_development_build_command(cp, up, &partial);
		change_verbose
		(
			cp,
			"%s build complete",
			(partial.wl_nwords ? "partial" : "development")
		);
		if
		(
			pconf_data->create_symlinks_before_build
		&&
			pconf_data->remove_symlinks_after_build
		)
			change_remove_symlinks_to_baseline(cp, up);
	}

	/*
	 * Update change data with result of build.
	 * (This will be used when validating developer sign off.)
	 * Release advisory write lock on row of change table.
	 */
	trace(("mark\n"));
	if (!partial.wl_nwords)
	{
		change_cstate_write(cp);
		commit();
		lock_release();
	}
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	build
 *
 * SYNOPSIS
 *	void build(void);
 *
 * DESCRIPTION
 *	The build function is used to
 *	dispatch the 'aegis -Build' command to the relevant functionality.
 *	Where it goes depends on the command line.
 */

void
build()
{
	trace(("build()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		build_main();
		break;

	case arglex_token_help:
		build_help();
		break;

	case arglex_token_list:
		build_list();
		break;
	}
	trace((/*{*/"}\n"));
}
