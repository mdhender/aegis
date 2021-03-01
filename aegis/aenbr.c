/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995, 1996, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to implement new branch
 */

#include <ac/stdio.h>

#include <ael/change/changes.h>
#include <aenbr.h>
#include <arglex2.h>
#include <change.h>
#include <change_bran.h>
#include <commit.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project_hist.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void new_branch_usage _((void));

static void
new_branch_usage()
{
	char	*progname;

	progname = progname_get();
	fprintf(stderr, "usage: %s -New_BRanch [ <option>... ]\n", progname);
	fprintf
	(
		
		stderr,
		"       %s -New_BRanch -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -New_BRanch -Help\n", progname);
	quit(1);
}


static void new_branch_help _((void));

static void
new_branch_help()
{
	help("aenbr", new_branch_usage);
}


static void new_branch_list _((void));

static void
new_branch_list()
{
	string_ty	*project_name;

	trace(("new_chane_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_branch_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, new_branch_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, new_branch_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_changes(project_name, 0);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static string_ty *branch_description_invent _((project_ty *));

static string_ty *
branch_description_invent(pp)
	project_ty	*pp;
{
	string_ty	*s1;
	string_ty	*s2;
	size_t		len;
	string_ty	*result;

	s2 = project_version_short_get(pp);
	while (pp->parent)
		pp = pp->parent;
	s1 = project_description_get(pp);
	if (s1->str_length == 0)
	{
		str_free(s2);
		return str_copy(s1);
	}
	for (len = s1->str_length; len && s1->str_text[len - 1] == '.'; --len)
		;
	result = str_format("%.*S, branch %S.", (int)len, s1, s2);
	/* do not free s1 */
	str_free(s2);
	return result;
}


project_ty *
new_branch_internals(up, ppp, change_number, devdir)
	user_ty		*up;
	project_ty	*ppp;
	long		change_number;
	string_ty	*devdir;
{
	cstate		cstate_data;
	cstate_history	history_data;
	change_ty	*cp;
	long		j;
	pconf		pconf_data;
	string_ty	*s;
	project_ty	*pp;

	/*
	 * On entry it is assumed that you have
	 * a project state lock, to create the branches
	 */
	trace(("new_branch_internals(ppp = %8.8lX, change_number = %ld)\n{\n"/*}*/,
		(long)ppp, change_number));

	/*
	 * create the new branch.
	 * validity of the branch number was checked elsewhere.
	 */
	cp = change_alloc(ppp, change_number);
	change_bind_new(cp);
	change_branch_new(cp);
	pp = project_bind_branch(ppp, cp);

	/*
	 * The copyright years are not copied from the parent, because
	 * they are updated by integrations.  The copyright years thus
	 * represent the copyright years for the changes made by the
	 * branch.
	 *
	 * However, when copyright years are calculated for ``ael
	 * version'' the branch's copyright years (actually, all
	 * ancestors') are included.
	 */

	/*
	 * Create the change (branch) directory.
	 *
	 * It is in the same place as the rest of the project, unless
	 * otherwise specified.  This makes it easy to collect the whole
	 * project tree, branches, info and all, in a simple tar or cpio
	 * command for archiving or moving.
	 */
	if (!devdir)
	{
		sub_context_ty	*scp;

		devdir =
			str_format
			(
				"%S/branch.%ld",
				project_top_path_get(ppp, 0),
				magic_zero_decode(change_number)
			);
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", devdir);
		change_verbose(cp, scp, i18n("development directory \"$filename\""));
		sub_context_delete(scp);
	}
	change_development_directory_set(cp, devdir);

	/*
	 * Create the development directory.
	 *
	 * In a branch, the development directory contains
	 *	(a) a directory called "baseline" for the branch's baseline
	 *	(b) directories called "delta.*" for integrations, later
	 *	(c) directories called "branch.*" for branches, later
	 *
	 * A branch's development directory is owned by the project
	 * owner, not the creating user.
	 *
	 * The "info" and "history" directories are only present in the
	 * development directory of the trunk.  DO NOT create them here.
	 */
	s = str_format("%S/baseline", devdir);
	project_become(pp);
	os_mkdir(devdir, 02755);
	os_mkdir(s, 02755);
	undo_rmdir_errok(s);
	undo_rmdir_errok(devdir);
	project_become_undo();
	str_free(s);

	/*
	 * Clear the time fields.
	 */
	change_build_times_clear(cp);

	/*
	 * many of the branch attributes are inherited from the parent
	 */
	change_branch_umask_set(cp, project_umask_get(ppp));
	change_branch_developer_may_review_set
	(
		cp,
		project_developer_may_review_get(ppp)
	);
	change_branch_developer_may_integrate_set
	(
		cp,
		project_developer_may_integrate_get(ppp)
	);
	change_branch_reviewer_may_integrate_set
	(
		cp,
		project_reviewer_may_integrate_get(ppp)
	);
	change_branch_developers_may_create_changes_set
	(
		cp,
		project_developers_may_create_changes_get(ppp)
	);
	change_branch_forced_develop_begin_notify_command_set
	(
		cp,
		project_forced_develop_begin_notify_command_get(ppp)
	);
	change_branch_develop_end_notify_command_set
	(
		cp,
		project_develop_end_notify_command_get(ppp)
	);
	change_branch_develop_end_undo_notify_command_set
	(
		cp,
		project_develop_end_undo_notify_command_get(ppp)
	);
	change_branch_review_pass_notify_command_set
	(
		cp,
		project_review_pass_notify_command_get(ppp)
	);
	change_branch_review_pass_undo_notify_command_set
	(
		cp,
		project_review_pass_undo_notify_command_get(ppp)
	);
	change_branch_review_fail_notify_command_set
	(
		cp,
		project_review_fail_notify_command_get(ppp)
	);
	change_branch_integrate_pass_notify_command_set
	(
		cp,
		project_integrate_pass_notify_command_get(ppp)
	);
	change_branch_integrate_fail_notify_command_set
	(
		cp,
		project_integrate_fail_notify_command_get(ppp)
	);
	change_branch_default_test_exemption_set
	(
		cp,
		project_default_test_exemption_get(ppp)
	);
	change_branch_default_development_directory_set
	(
		cp,
		project_default_development_directory_get(ppp)
	);

	/*
	 * staff lists are inherited from the parent
	 */
	for (j = 0; ; ++j)
	{
		s = project_administrator_nth(ppp, j);
		if (!s)
			break;
		change_branch_administrator_add(cp, s);
	}
	for (j = 0; ; ++j)
	{
		s = project_developer_nth(ppp, j);
		if (!s)
			break;
		change_branch_developer_add(cp, s);
	}
	for (j = 0; ; ++j)
	{
		s = project_reviewer_nth(ppp, j);
		if (!s)
			break;
		change_branch_reviewer_add(cp, s);
	}
	for (j = 0; ; ++j)
	{
		s = project_integrator_nth(ppp, j);
		if (!s)
			break;
		change_branch_integrator_add(cp, s);
	}
	change_branch_minimum_change_number_set
	(
		cp,
		project_minimum_change_number_get(ppp)
	);
	change_branch_reuse_change_numbers_set
	(
		cp,
		project_reuse_change_numbers_get(ppp)
	);
	change_branch_minimum_branch_number_set
	(
		cp,
		project_minimum_branch_number_get(ppp)
	);
	change_branch_skip_unlucky_set
	(
		cp,
		project_skip_unlucky_get(ppp)
	);

	/*
	 * architecture is inherited from parent
	 */
	pconf_data = change_pconf_get(cp, 0);
	assert(pconf_data->architecture);
	change_architecture_clear(cp);
	for (j = 0; j < pconf_data->architecture->length; ++j)
	{
		change_architecture_add
		(
			cp,
			pconf_data->architecture->list[j]->name
		);
	}

	/*
	 * create the change history,
	 * and advance the change to 'being developed'
	 */
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_new_change;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_develop_begin;
	cstate_data = change_cstate_get(cp);
	cstate_data->state = cstate_state_being_developed;

	/*
	 * set the attributes
	 */
	cstate_data->description = branch_description_invent(pp);
	cstate_data->brief_description = str_copy(cstate_data->description);
	cstate_data->cause = change_cause_internal_enhancement;
	cstate_data->test_exempt = 1;
	cstate_data->test_baseline_exempt = 1;
	cstate_data->regression_test_exempt = 1;

	/*
	 * Add the change to the list of existing changes
	 */
	project_change_append(ppp, change_number, 1);
	trace(("return %8.8lX;\n", (long)pp));
	trace((/*{*/"}\n"));
	return pp;
}


static void new_branch_main _((void));

static void
new_branch_main()
{
	string_ty	*project_name;
	project_ty	*pp;
	project_ty	*bp;
	long		change_number;
	user_ty		*up;
	string_ty	*devdir;
	char		*output;

	trace(("new_branch_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	devdir = 0;
	output = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_branch_usage);
			continue;

		case arglex_token_branch:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_branch, new_branch_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_branch, new_branch_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "Number", "%ld", change_number);
				fatal_intl(scp, i18n("branch $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (project_name)
				duplicate_option(new_branch_usage);
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, new_branch_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_directory:
			if (arglex() != arglex_token_string)
				option_needs_dir(arglex_token_directory, new_branch_usage);
			if (devdir)
				duplicate_option_by_name(arglex_token_directory, new_branch_usage);
			/*
			 * To cope with automounters, directories are stored as
			 * given, or are derived from the home directory in the
			 * passwd file.  Within aegis, pathnames have their
			 * symbolic links resolved, and any comparison of paths
			 * is done on this "system idea" of the pathname.
			 */
			devdir = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(new_branch_usage);
			break;

		case arglex_token_output:
			if (output)
				duplicate_option(new_branch_usage);
			switch (arglex())
			{
			default:
				option_needs_file(arglex_token_output, new_branch_usage);
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
	if (change_number && output)
	{
		mutually_exclusive_options
		(
			arglex_token_change,
			arglex_token_output,
			new_branch_usage
		);
	}

	/*
	 * locate project data
	 */
	if (!project_name)
		fatal_intl(0, i18n("no project name"));
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
	 * Lock the project state file and the user state file.  There
	 * is no need to lock the project state file of the new branch
	 * as that file does not exist.
	 */
	project_pstate_lock_prepare(pp);
	lock_take();

	/*
	 * it is an error if
	 * the user is not an administrator for the project.
	 */
	if (!project_administrator_query(pp, user_name(up)))
		project_fatal(pp, 0, i18n("not an administrator"));

	/*
	 * Add another row to the change table.
	 */
	if (!change_number)
		change_number = project_next_change_number(pp, 0);
	else
	{
		if (project_change_number_in_use(pp, change_number))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Number", "%ld", magic_zero_decode(change_number));
			project_fatal(pp, scp, i18n("branch $number used"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
	}

	/*
	 * create the new branch
	 */
	bp = new_branch_internals(up, pp, change_number, devdir);

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

			fn = str_from_c(output);
			user_become(up);
			file_from_string(fn, content, 0644);
			user_become_undo();
			str_free(fn);
		}
		else
			cat_string_to_stdout(content);
		str_free(content);
	}

	/*
	 * Write out the various files
	 */
	change_cstate_write(bp->pcp);
	project_pstate_write(pp);

	/*
	 * release the locks
	 */
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	project_verbose(bp, 0, i18n("new branch complete"));
	project_free(bp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
new_branch()
{
	trace(("new_branch()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		new_branch_main();
		break;

	case arglex_token_help:
		new_branch_help();
		break;

	case arglex_token_list:
		new_branch_list();
		break;
	}
	trace((/*{*/"}\n"));
}
