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
 * MANIFEST: functions for implementing integrate pass
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	fprintf(stderr, "usage: %s -Integrate_PASS [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_PASS -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_PASS -Help\n", progname);
	quit(1);
}


static void integrate_pass_help _((void));

static void
integrate_pass_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Integrate_PASS - pass a change integration",
"",
"SYNOPSIS",
"	%s -Integrate_PASS [ <option>... ]",
"	%s -Integrate_PASS -List [ <option>... ]",
"	%s -Integrate_PASS -Help",
"",
"DESCRIPTION",
"	The %s -Integrate_PASS command is used to notify %s",
"	that a change has passed integration.",
"",
"	The change is advanced from the 'being_integrated' state",
"	to the 'completed' state.  The integration directory is",
"	renamed as the baseline directory, and the baseline",
"	directory is deleted.  The change is no longer assigned",
"	to the current user.",
"",
"	While there is a build in progress for any change in a",
"	project, an integrate pass for the project will wait until",
"	all the builds are completed before starting.  This is to",
"	ensure that the baseline is consistent for the entire build.",
"	Similarly, while an integrate pass is in progress for a",
"	project, any builds will wait until it is completed before",
"	starting.",
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
"	csh%%	alias aeip '%s -ip \\!* -v'",
"	sh$	aeip(){%s -ip $* -v}",
"",
"ERRORS",
"	It is an error if the change is not assigned to the",
"	current user.",
"	It is an error if The change is not in the",
"	'being_integrated' state.",
"	It is an error if there has been no successful '%s",
"	-Build' command for the integration.",
"	It is an error if there has been no successful '%s",
"	-Test' command for the integration.",
"	It is an error if there has been no successful '%s",
"	-Test -BaseLine' command for the integration.",
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
	pconf		pconf_data;
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
				fatal("duplicate %s option", arglex_value.alv_string);
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
	pconf_data = change_pconf_get(cp);

	/*
	 * It is an error if the change is not in the being_integrated state.
	 * It is an error if the change is not assigned to the current user.
	 * It is an error if the change has no current build.
	 * It is an error if the change has no current test pass.
	 * It is an error if the change has no current baseline test pass.
	 */
	if (cstate_data->state != cstate_state_being_integrated)
		change_fatal(cp, "not in 'being_integrated' state");
	nerr = 0;
	if (!str_equal(change_integrator_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
			"user \"%S\" is not the integrator",
			user_name(up)
		);
	}
	if (!cstate_data->build_time)
	{
		change_error
		(
			cp,
			"no current '%s -Build' registration",
			option_progname_get()
		);
		++nerr;
	}
	if (!cstate_data->test_exempt && !cstate_data->test_time)
	{
		change_error
		(
			cp,
			"no current '%s -Test' registration",
			option_progname_get()
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
			"no current '%s -Test -BaseLine' registration",
			option_progname_get()
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
			"no current '%s -Test -REGression' registration",
			option_progname_get()
		);
		++nerr;
	}
	if (nerr)
		quit(1);

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
		log_open(change_logfile_get(cp), pup);
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
	cstate_data->build_time = 0;
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;
	cstate_data->regression_test_time = 0;
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
