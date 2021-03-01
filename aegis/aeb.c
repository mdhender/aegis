/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-2001 Peter Miller;
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
 * MANIFEST: functions to perform development and integration builds
 */

#include <ac/errno.h>
#include <ac/libintl.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>

#include <aeb.h>
#include <ael/change/by_state.h>
#include <arglex2.h>
#include <col.h>
#include <commit.h>
#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <str_list.h>


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

	progname = progname_get();
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
	help("aeb", build_usage);
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
				option_needs_name(arglex_token_project, build_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, build_usage);
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
	pconf		pconf_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	log_style_ty	log_style;
	user_ty		*up;
	string_list_ty	partial;
	string_ty	*s1;
	string_ty	*s2;
	int		minimum;
	int		based;
	string_ty	*base;

	trace(("build_main()\n{\n"/*}*/));
	arglex();
	log_style = log_style_snuggle_default;
	project_name = 0;
	change_number = 0;
	minimum = 0;
	string_list_constructor(&partial);
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(build_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, build_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, build_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set_long(scp, "Number", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, build_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, build_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_file:
			if (arglex() != arglex_token_string)
				option_needs_files(arglex_token_file, build_usage);
			/* fall through... */

		case arglex_token_string:
			s2 = str_from_c(arglex_value.alv_string);
			string_list_append(&partial, s2);
			str_free(s2);
			break;

		case arglex_token_nolog:
			if (log_style == log_style_none)
				duplicate_option(build_usage);
			log_style = log_style_none;
			break;

		case arglex_token_minimum:
			if (minimum)
				duplicate_option(build_usage);
			minimum = 1;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(build_usage);
			break;

		case arglex_token_symbolic_links:
		case arglex_token_symbolic_links_not:
			user_symlink_pref_argument(build_usage);
			break;

		case arglex_token_base_relative:
		case arglex_token_current_relative:
			user_relative_filename_preference_argument(build_usage);
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
	 *
	 * Also take a read lock on the baseline, to ensure that it does
	 * not change (aeip) for the duration of the build.
	 */
	if (!partial.nstrings)
		change_cstate_lock_prepare(cp);
	project_baseline_read_lock_prepare(pp);
	lock_take();
	cstate_data = change_cstate_get(cp);

	/*
	 * Extract the appropriate row of the change table.
	 * It is an error if the change is not in the in-development state.
	 * It is an error if the change is not assigned to the current user.
	 * It is an error if the change has no files assigned.
	 */
	switch (cstate_data->state)
	{
	default:
		change_fatal(cp, 0, i18n("bad build state"));
		break;

	case cstate_state_being_developed:
		if (change_is_a_branch(cp))
			change_fatal(cp, 0, i18n("bad branch build"));
		if (!str_equal(change_developer_name(cp), user_name(up)))
			change_fatal(cp, 0, i18n("not developer"));
		if (!change_file_nth(cp, (size_t)0))
			change_fatal(cp, 0, i18n("no files"));
		break;

	case cstate_state_being_integrated:
		if (!str_equal(change_integrator_name(cp), user_name(up)))
			change_fatal(cp, 0, i18n("not integrator"));
		if (partial.nstrings)
			change_fatal(cp, 0, i18n("bad build, partial"));
		break;
	}

	/*
	 * Resolve relative filenames into project filenames.
	 * Do this after we know the change is in a buildable state.
	 */
	if (partial.nstrings)
	{
		string_list_ty	search_path;
		string_list_ty	wl2;
		size_t		j, k;

		/*
		 * Search path for resolving file names.
		 */
		change_search_path_get(cp, &search_path, 1);

		/*
		 * Find the base for relative filenames.
		 */
		based =
			(
				search_path.nstrings >= 1
			&&
				(
					user_relative_filename_preference
					(
						up,
						uconf_relative_filename_preference_base
					)
				==
					uconf_relative_filename_preference_base
				)
			);
		if (based)
			base = str_copy(search_path.string[0]);
		else
		{
			os_become_orig();
			base = os_curdir();
			os_become_undo();
		}

		/*
		 * resolve the path of each file
		 * 1.	the absolute path of the file name is obtained
		 * 2.	if the file is inside the development directory, ok
		 * 3.	if the file is inside the baseline, ok
		 * 4.	if neither, error
		 */
		string_list_constructor(&wl2);
		for (j = 0; j < partial.nstrings; ++j)
		{
			/*
			 * leave variable assignments alone
			 */
			s1 = partial.string[j];
			if (strchr(s1->str_text, '='))
			{
				string_list_append(&wl2, s1);
				continue;
			}

			/*
			 * resolve relative paths
			 */
			if (s1->str_text[0] == '/')
				s2 = str_copy(s1);
			else
				s2 = str_format("%S/%S", base, s1);
			user_become(up);
			s1 = os_pathname(s2, 0);
			user_become_undo();
			str_free(s2);
			s2 = 0;
			for (k = 0; k < search_path.nstrings; ++k)
			{
				s2 = os_below_dir(search_path.string[k], s1);
				if (s2)
					break;
			}
			str_free(s1);
			if (!s2)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set_string(scp, "File_Name", partial.string[j]);
				change_fatal(cp, scp, i18n("$filename unrelated"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}

			/*
			 * make sure it's unique
			 */
			if (string_list_member(&wl2, s2))
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set_string(scp, "File_Name", s2);
				change_fatal(cp, scp, i18n("too many $filename"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			else
				string_list_append(&wl2, s2);
			str_free(s2);
		}
		string_list_destructor(&partial);
		partial = wl2;
	}

	/*
	 * It is an error if the change attributes include architectures
	 * not in the project.
	 */
	change_check_architectures(cp);

	/*
	 * Update the time the build was done.
	 * This will not be written out if the build fails.
	 */
	os_throttle();
	change_build_time_set(cp);

	/*
	 * get the command to execute
	 *  1. if the change is editing config, use that
	 *  2. if the baseline contains config, use that
	 *  3. error if can't find one (DON'T look for file existence)
	 */
	pconf_data = change_pconf_get(cp, 1);

	/*
	 * make sure the -MINIMum option means something
	 */
	if (minimum && !pconf_data->create_symlinks_before_build)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_charstar(scp, "Name", arglex_token_name(arglex_token_minimum));
		change_fatal(cp, scp, i18n("$name option not meaningful"));
		sub_context_delete(scp);
	}

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
		user_ty		*pup;

		pup = project_user(pp);
		log_open(change_logfile_get(cp), pup, log_style);

		if
		(
			pp->parent
		&&
			pconf_data->create_symlinks_before_integration_build
		)
			change_create_symlinks_to_baseline(cp, pp->parent, pup, minimum);

		change_verbose(cp, 0, i18n("integration build started"));
		change_run_build_command(cp);
		change_verbose(cp, 0, i18n("integration build complete"));

		if
		(
			pp->parent
		&&
			pconf_data->create_symlinks_before_integration_build
		&&
			pconf_data->remove_symlinks_after_integration_build
		)
		{
			change_remove_symlinks_to_baseline(cp, pp->parent, pup);
		}
		user_free(pup);
	}
	else
	{
		log_open(change_logfile_get(cp), up, log_style);
		if (pconf_data->create_symlinks_before_build)
		{
			int	verify_dflt;

			verify_dflt =
				(
					pconf_data->remove_symlinks_after_build
				||
				      change_run_project_file_command_needed(cp)
				);
			if (user_symlink_pref(up, verify_dflt))
				change_create_symlinks_to_baseline(cp, pp, up, minimum);
		}
		if (partial.nstrings)
			change_verbose(cp, 0, i18n("partial build started"));
		else
			change_verbose(cp, 0, i18n("development build started"));
		change_run_project_file_command(cp, up);
		change_run_development_build_command(cp, up, &partial);
		if (partial.nstrings)
			change_verbose(cp, 0, i18n("partial build complete"));
		else
			change_verbose(cp, 0, i18n("development build complete"));
		if
		(
			pconf_data->create_symlinks_before_build
		&&
			pconf_data->remove_symlinks_after_build
		)
			change_remove_symlinks_to_baseline(cp, pp, up);
	}

	/*
	 * Update change data with result of build.
	 * (This will be used when validating developer sign off.)
	 * Release advisory write lock on row of change table.
	 */
	if (!partial.nstrings)
	{
		change_file_list_metrics_check(cp);
		change_cstate_write(cp);
		commit();
	}
	lock_release();
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
	static arglex_dispatch_ty dispatch[] =
	{
		{ arglex_token_help,            build_help,         },
		{ arglex_token_list,            build_list,         },
	};

	trace(("build()\n{\n"));
	arglex_dispatch(dispatch, SIZEOF(dispatch), build_main);
	trace(("}\n"));
}
