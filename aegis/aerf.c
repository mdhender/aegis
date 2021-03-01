/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to impliment review fail
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <ael.h>
#include <aerf.h>
#include <arglex2.h>
#include <change_file.h>
#include <commit.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <mem.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project_file.h>
#include <project_hist.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void review_fail_usage _((void));

static void
review_fail_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf(stderr, "usage: %s -Review_FAIL -File <reason-file> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Review_FAIL -REAson '<text>' [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Review_FAIL -Edit [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Review_FAIL -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Review_FAIL -Help\n", progname);
	quit(1);
}


static void review_fail_help _((void));

static void
review_fail_help()
{
	help("aerf", review_fail_usage);
}


static void review_fail_list _((void (*usage)(void)));

static void
review_fail_list(usage)
	void		(*usage)_((void));
{
	string_ty	*project_name;

	trace(("review_fail_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, usage);
			/* fall through... */

		case arglex_token_string:
			if (project_name)
				duplicate_option_by_name(arglex_token_project, usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_changes_in_state_mask
	(
		project_name,
		1 << cstate_state_being_reviewed
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void check_permissions _((change_ty *, user_ty *));

static void
check_permissions(cp, up)
	change_ty	*cp;
	user_ty		*up;
{
	cstate		cstate_data;
	project_ty	*pp;

	cstate_data = change_cstate_get(cp);
	pp = cp->pp;

	/*
	 * it is an error if the change is not in the 'being_reviewed' state.
	 */
	if (cstate_data->state != cstate_state_being_reviewed)
		change_fatal(cp, 0, i18n("bad rf state"));
	if (!project_reviewer_query(pp, user_name(up)))
		project_fatal(pp, 0, i18n("not a reviewer"));
	if
	(
		!project_developer_may_review_get(pp)
	&&
		str_equal(change_developer_name(cp), user_name(up))
	)
	{
		change_fatal(cp, 0, i18n("developer may not review"));
	}
}


static void review_fail_main _((void));

static void
review_fail_main()
{
	sub_context_ty	*scp;
	cstate		cstate_data;
	cstate_history	history_data;
	string_ty	*comment = 0;
	char		*reason = 0;
	int		j;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	user_ty		*devup;
	edit_ty		edit;

	trace(("review_fail_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	edit = edit_not_set;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(review_fail_usage);
			continue;

		case arglex_token_string:
			scp = sub_context_new();
			sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_file));
			error_intl(scp, i18n("warning: use $name option"));
			sub_context_delete(scp);
			if (comment)
				fatal_intl(0, i18n("too many files"));
			goto read_input_file;

		case arglex_token_file:
			if (comment)
				duplicate_option(review_fail_usage);
			if (arglex() != arglex_token_string)
				option_needs_file(arglex_token_file, review_fail_usage);
			read_input_file:
			os_become_orig();
			comment = read_whole_file(arglex_value.alv_string);
			os_become_undo();
			break;

		case arglex_token_reason:
			if (reason)
				duplicate_option(review_fail_usage);
			if (arglex() != arglex_token_string)
				option_needs_string(arglex_token_reason, review_fail_usage);
			reason = arglex_value.alv_string;
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, review_fail_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, review_fail_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				scp = sub_context_new();
				sub_var_set(scp, "Number", "%ld", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (project_name)
				duplicate_option(review_fail_usage);
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, review_fail_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_edit:
			if (edit == edit_foreground)
				duplicate_option(review_fail_usage);
			if (edit != edit_not_set)
			{
				too_many_edits:
				mutually_exclusive_options
				(
					arglex_token_edit,
					arglex_token_edit_bg,
					review_fail_usage
				);
			}
			edit = edit_foreground;
			break;

		case arglex_token_edit_bg:
			if (edit == edit_background)
				duplicate_option(review_fail_usage);
			if (edit != edit_not_set)
				goto too_many_edits;
			edit = edit_background;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(review_fail_usage);
			break;
		}
		arglex();
	}
	if (comment && reason)
	{
		mutually_exclusive_options
		(
			arglex_token_file,
			arglex_token_reason,
			review_fail_usage
		);
	}
	if (edit != edit_not_set && (comment || reason))
	{
		mutually_exclusive_options
		(
			(
				edit == edit_foreground
			?
				arglex_token_edit
			:
				arglex_token_edit_bg
			),
			(comment ? arglex_token_file : arglex_token_reason),
			review_fail_usage
		);
	}
	if (edit == edit_not_set && !(comment || reason))
	{
		scp = sub_context_new();
		sub_var_set(scp, "Name1", arglex_token_name(arglex_token_file));
		sub_var_set(scp, "Name2", arglex_token_name(arglex_token_edit));
		error_intl(scp, i18n("warning: no $name1, assuming $name2"));
		sub_context_delete(scp);
		edit = edit_foreground;
	}
	if (reason)
		comment = str_from_c(reason);

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
	 * create the commant, if necessary
	 * check permissions first
	 */
	if (edit != edit_not_set)
	{
		check_permissions(cp, up);
		comment = os_edit_new(edit);
	}

	/*
	 * lock the change for writing
	 */
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	lock_prepare_ustate_all(0, 0); /* we don't know which user until later */
	lock_take();
	cstate_data = change_cstate_get(cp);

	/*
	 * check they are allowed to
	 * (even if edited, may have changed while editing)
	 */
	check_permissions(cp, up);

	/*
	 * change the state
	 * remember who reviewed it
	 * add to the change's history
	 */
	cstate_data->state = cstate_state_being_developed;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_review_fail;
	history_data->why = comment;
	change_build_times_clear(cp);

	/*
	 * add it back into the user's change list
	 */
	devup = user_symbolic(pp, change_developer_name(cp));
	user_own_add(devup, project_name_get(pp), change_number);

	/*
	 * go through the files in the change and unlock them
	 * in the baseline
	 */
	for (j = 0; ; ++j)
	{
		fstate_src	c_src_data;
		fstate_src	p_src_data;

		c_src_data = change_file_nth(cp, j);
		if (!c_src_data)
			break;
		p_src_data = project_file_find(pp, c_src_data->file_name);
		if (!p_src_data)
			continue;
		p_src_data->locked_by = 0;

		/*
		 * Remove the file if it is about_to_be_created
		 * by the change we are rescinding.
		 */
		if
		(
			p_src_data->about_to_be_created_by
		||
			p_src_data->about_to_be_copied_by
		)
		{
			assert(!p_src_data->about_to_be_created_by || p_src_data->about_to_be_created_by == change_number);
			assert(!p_src_data->about_to_be_copied_by || p_src_data->about_to_be_copied_by == change_number);
			if (p_src_data->deleted_by)
			{
				assert(!p_src_data->about_to_be_copied_by);
				p_src_data->about_to_be_created_by = 0;
			}
			else
				project_file_remove(pp, c_src_data->file_name);
		}
	}

	/*
	 * write out the data and release the locks
	 */
	project_pstate_write(pp);
	change_cstate_write(cp);
	user_ustate_write(devup);
	commit();
	lock_release();

	/*
	 * run the notify command
	 */
	change_run_review_fail_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, 0, i18n("review fail complete"));
	change_free(cp);
	project_free(pp);
	user_free(up);
	user_free(devup);
	trace((/*{*/"}\n"));
}


void
review_fail()
{
	trace(("review_fail()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		review_fail_main();
		break;

	case arglex_token_help:
		review_fail_help();
		break;

	case arglex_token_list:
		review_fail_list(review_fail_usage);
		break;
	}
	trace((/*{*/"}\n"));
}
