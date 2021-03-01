/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1999, 2001-2003 Peter Miller;
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
 * MANIFEST: functions to implement review pass
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ael/change/by_state.h>
#include <aerp.h>
#include <arglex2.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <commit.h>
#include <dir.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <mem.h>
#include <os.h>
#include <pattr.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <project/history.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void
review_pass_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -Review_PASS <change_number> [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -Review_PASS -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Review_PASS -Help\n", progname);
    quit(1);
}


static void
review_pass_help(void)
{
    help("aerpass", review_pass_usage);
}


static void
review_pass_list(void)
{
    string_ty	    *project_name;

    trace(("review_pass_list()\n{\n"));
    project_name = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(review_pass_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    /* fall through... */

	case arglex_token_string:
	    arglex_parse_project(&project_name, review_pass_usage);
	    continue;
	}
	arglex();
    }
    list_changes_in_state_mask(project_name, 1 << cstate_state_being_reviewed);
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


static void
check_permissions(change_ty *cp, user_ty *up)
{
    cstate_ty	    *cstate_data;
    project_ty	    *pp;
    cstate_history_ty *hp;

    cstate_data = change_cstate_get(cp);

    /*
     * it is an error if the change is not in the 'being_reviewed' state.
     */
    if (cstate_data->state != cstate_state_being_reviewed)
	change_fatal(cp, 0, i18n("bad rp state"));
    assert(cstate_data->history->length >= 3);
    hp = cstate_data->history->list[cstate_data->history->length - 1];
    if (hp->what == cstate_history_what_review_begin)
    {
	if (!str_equal(change_reviewer_name(cp), user_name(up)))
	    change_fatal(cp, 0, i18n("not reviewer"));
    }
    else
    {
	assert(hp->what == cstate_history_what_develop_end);
	pp = cp->pp;
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
}


static void
review_pass_main(void)
{
    cstate_ty	    *cstate_data;
    cstate_history_ty *history_data;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    user_ty	    *up;
    long	    j;
    string_ty	    *comment =	    0;
    const char      *reason =	    0;
    edit_ty	    edit;

    trace(("review_pass_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    edit = edit_not_set;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(review_pass_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    /* fall through... */

	case arglex_token_number:
	    arglex_parse_change
	    (
		&project_name,
		&change_number,
		review_pass_usage
	    );
	    continue;

	case arglex_token_project:
	    arglex();
	    /* fall through... */

	case arglex_token_string:
	    arglex_parse_project(&project_name, review_pass_usage);
	    continue;

	case arglex_token_file:
	    if (comment)
		duplicate_option(review_pass_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, review_pass_usage);
		/*NOTREACHED*/

	    case arglex_token_string:
		{
		    string_ty       *s;

		    os_become_orig();
		    s = str_from_c(arglex_value.alv_string);
		    comment = read_whole_file(s);
		    str_free(s);
		    os_become_undo();
		}
		break;

	    case arglex_token_stdio:
		os_become_orig();
		comment = read_whole_file((string_ty *)0);
		os_become_undo();
		break;
	    }
	    assert(comment);
	    break;

	case arglex_token_reason:
	    if (reason)
		duplicate_option(review_pass_usage);
	    if (arglex() != arglex_token_string)
		option_needs_string(arglex_token_reason, review_pass_usage);
	    reason = arglex_value.alv_string;
	    break;

	case arglex_token_edit:
	    if (edit == edit_foreground)
		duplicate_option(review_pass_usage);
	    if (edit != edit_not_set)
	    {
		too_many_edits:
		mutually_exclusive_options
		(
		    arglex_token_edit,
		    arglex_token_edit_bg,
		    review_pass_usage
		);
	    }
	    edit = edit_foreground;
	    break;

	case arglex_token_edit_bg:
	    if (edit == edit_background)
		duplicate_option(review_pass_usage);
	    if (edit != edit_not_set)
		goto too_many_edits;
	    edit = edit_background;
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(review_pass_usage);
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
	    review_pass_usage
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
	    review_pass_usage
	);
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
     * create the comment, if necessary
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
    change_cstate_lock_prepare(cp);
    lock_take();
    cstate_data = change_cstate_get(cp);

    /*
     * it is an error if the change is not in the 'being_reviewed' state.
     */
    check_permissions(cp, up);

    /*
     * change the state
     * remember who reviewed it
     * add to the change's history
     */
    cstate_data->state = cstate_state_awaiting_integration;
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_review_pass;
    history_data->why = comment;

    /*
     * It is an error if any of the change files have been tampered
     * with, or any of the difference files.
     */
    for (j = 0;; ++j)
    {
	fstate_src_ty   *src_data;
	string_ty	*path;
	string_ty	*path_d;
	int		same;
	int		file_required;
	int		diff_file_required;

	src_data = change_file_nth(cp, j);
	if (!src_data)
	    break;

	file_required = 1;
	diff_file_required = 1;
	switch (src_data->usage)
	{
	case file_usage_source:
	case file_usage_config:
	case file_usage_test:
	case file_usage_manual_test:
	    break;

	case file_usage_build:
	    file_required = 0;
	    diff_file_required = 0;
	    break;
	}
	switch (src_data->action)
	{
	case file_action_create:
	case file_action_modify:
	    break;

	case file_action_insulate:
	    assert(0);
	    break;

	case file_action_transparent:
	    if (change_was_a_branch(cp))
	    {
		file_required = 0;
		diff_file_required = 0;
	    }
	    break;

	case file_action_remove:
	    file_required = 0;

	    /*
	     * the removed half of a move is not differenced
	     */
	    if (src_data->move && change_file_find(cp, src_data->move))
		diff_file_required = 0;
	    break;
	}

	path = change_file_path(cp, src_data->file_name);
	if (file_required)
	{
	    user_become(up);
	    same = change_fingerprint_same(src_data->file_fp, path, 0);
	    user_become_undo();
	    if (!same)
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", src_data->file_name);
		change_fatal(cp, scp, i18n("$filename altered"));
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	}

	path_d = str_format("%S,D", path);
	if (diff_file_required)
	{
	    user_become(up);
	    same = change_fingerprint_same(src_data->diff_file_fp, path_d, 0);
	    user_become_undo();
	    if (!same)
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_format
		(
		    scp,
		    "File_Name",
		    "%S,D",
		    src_data->file_name
		);
		change_fatal(cp, scp, i18n("$filename altered"));
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	}
	str_free(path);
	str_free(path_d);
    }

    /*
     * write out the data and release the locks
     */
    change_cstate_write(cp);
    commit();
    lock_release();

    /*
     * run the notify command
     */
    change_run_review_pass_notify_command(cp);

    /*
     * verbose success message
     */
    change_verbose(cp, 0, i18n("review pass complete"));
    change_free(cp);
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}


void
review_pass(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, review_pass_help, },
	{arglex_token_list, review_pass_list, },
    };

    trace(("review_pass()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), review_pass_main);
    trace(("}\n"));
}
