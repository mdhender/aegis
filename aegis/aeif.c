/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-2003 Peter Miller;
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
 * MANIFEST: functions for implementing integrate fail
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <aeif.h>
#include <ael/change/by_state.h>
#include <arglex2.h>
#include <commit.h>
#include <change/branch.h>
#include <change/develop_direct/read_write.h>
#include <change/file.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <progname.h>
#include <os.h>
#include <project.h>
#include <project/file.h>
#include <project/history.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void
integrate_fail_usage(void)
{
    char	    *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -Integrate_FAIL -File <reason-file> [ <option>... ]\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -Integrate_FAIL -REAson '<text>' [ <option>... ]\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -Integrate_FAIL -Edit [ <option>... ]\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -Integrate_FAIL -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -Integrate_FAIL -Help\n", progname);
    quit(1);
}


static void
integrate_fail_help(void)
{
    help("aeifail", integrate_fail_usage);
}


static void
integrate_fail_list(void)
{
    string_ty	    *project_name;

    trace(("integrate_fail_list()\n{\n"));
    arglex();
    project_name = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(integrate_fail_usage);
	    continue;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, integrate_fail_usage);
	    if (project_name)
	    {
		duplicate_option_by_name
		(
		    arglex_token_project,
		    integrate_fail_usage
		);
	    }
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
    trace(("}\n"));
}


static void
check_directory(change_ty *cp)
{
    string_ty	    *dir;

    dir = change_integration_directory_get(cp, 1);
    os_become_orig();
    if (os_below_dir(dir, os_curdir()))
	change_fatal(cp, 0, i18n("leave int dir"));
    os_become_undo();
}


static void
check_permissions(change_ty *cp, user_ty *up)
{
    cstate	    cstate_data;

    cstate_data = change_cstate_get(cp);

    /*
     * it is an error if the change is not in the 'being_integrated' state.
     */
    if (cstate_data->state != cstate_state_being_integrated)
	change_fatal(cp, 0, i18n("bad if state"));
    if (!str_equal(change_integrator_name(cp), user_name(up)))
	change_fatal(cp, 0, i18n("not integrator"));
}


static void
integrate_fail_main(void)
{
    string_ty	    *s;
    sub_context_ty  *scp;
    cstate	    cstate_data;
    cstate_history  history_data;
    string_ty	    *comment =	    0;
    char	    *reason =	    0;
    string_ty	    *rev_name;
    string_ty	    *int_name;
    string_ty	    *dir;
    int		    j;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    user_ty	    *up;
    user_ty	    *devup;
    edit_ty	    edit;

    trace(("integrate_fail_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    edit = edit_not_set;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(integrate_fail_usage);
	    continue;

	case arglex_token_string:
	    scp = sub_context_new();
	    sub_var_set_charstar
	    (
		scp,
		"Name",
		arglex_token_name(arglex_token_file)
	    );
	    error_intl(scp, i18n("warning: use $name option"));
	    sub_context_delete(scp);
	    if (comment)
		fatal_too_many_files();
	    goto read_reason_file;

	case arglex_token_file:
	    if (comment)
		duplicate_option(integrate_fail_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, integrate_fail_usage);
		/*NOTREACHED*/

	    case arglex_token_string:
		read_reason_file:
		os_become_orig();
		s = str_from_c(arglex_value.alv_string);
		comment = read_whole_file(s);
		str_free(s);
		os_become_undo();
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
		duplicate_option(integrate_fail_usage);
	    if (arglex() != arglex_token_string)
		option_needs_string(arglex_token_reason, integrate_fail_usage);
	    reason = arglex_value.alv_string;
	    break;

	case arglex_token_change:
	    if (arglex() != arglex_token_number)
		option_needs_number(arglex_token_change, integrate_fail_usage);
	    /* fall through... */

	case arglex_token_number:
	    if (change_number)
	    {
		duplicate_option_by_name
		(
		    arglex_token_change,
		    integrate_fail_usage
		);
	    }
	    change_number = arglex_value.alv_number;
	    if (change_number == 0)
		change_number = MAGIC_ZERO;
	    else if (change_number < 1)
	    {
		scp = sub_context_new();
		sub_var_set_long(scp, "Number", change_number);
		fatal_intl(scp, i18n("change $number out of range"));
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    break;

	case arglex_token_project:
	    if (project_name)
		duplicate_option(integrate_fail_usage);
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, integrate_fail_usage);
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_edit:
	    if (edit == edit_foreground)
		duplicate_option(integrate_fail_usage);
	    if (edit != edit_not_set)
	    {
		too_many_edits:
		mutually_exclusive_options
		(
		    arglex_token_edit,
		    arglex_token_edit_bg,
		    integrate_fail_usage
		);
	    }
	    edit = edit_foreground;
	    break;

	case arglex_token_edit_bg:
	    if (edit == edit_background)
		duplicate_option(integrate_fail_usage);
	    if (edit != edit_not_set)
		goto too_many_edits;
	    edit = edit_background;
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(integrate_fail_usage);
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
	    integrate_fail_usage
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
	    integrate_fail_usage
	);
    }
    if (edit == edit_not_set && !(comment || reason))
    {
	scp = sub_context_new();
	sub_var_set_charstar
	(
	    scp,
	    "Name1",
	    arglex_token_name(arglex_token_file)
	);
	sub_var_set_charstar
	(
	    scp,
	    "Name2",
	    arglex_token_name(arglex_token_edit)
	);
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
     * create the comments, if required
     */
    if (edit != edit_not_set)
    {
	/*
	 * make sure they are not in the integration directory,
	 * to avoid a wasted edit
	 */
	check_directory(cp);

	/*
	 * make sure they are allowed to first,
	 * to avoid a wasted edit
	 */
	check_permissions(cp, up);
	comment = os_edit_new(edit);
    }

    /*
     * lock the change for writing
     */
    project_pstate_lock_prepare(pp);
    change_cstate_lock_prepare(cp);
    lock_prepare_ustate_all(0, 0); /* we don't know which users until later */
    lock_take();
    cstate_data = change_cstate_get(cp);

    /*
     * make sure they are allowed to
     * (even if edited, could have changed during edit)
     */
    check_permissions(cp, up);

    /*
     * Change the state.
     * Add to the change's history.
     */
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_integrate_fail;
    history_data->why = comment;
    change_build_times_clear(cp);
    rev_name = change_reviewer_name(cp);
    int_name = change_integrator_name(cp);
    cstate_data->delta_number = 0;
    dir = str_copy(change_integration_directory_get(cp, 1));
    change_integration_directory_clear(cp);
    cstate_data->state = cstate_state_being_developed;
    cstate_data->minimum_integration = 0;

    /*
     * Complain if they are in the integration directory,
     * because the rmdir at the end can't then run to completion.
     */
    check_directory(cp);

    /*
     * note that the project has no current integration
     */
    project_current_integration_set(pp, 0);

    /*
     * Remove it from the integrator's change list, and
     * add it back into the developer's change list.
     */
    user_own_remove(up, project_name_get(pp), change_number);
    devup = user_symbolic(pp, change_developer_name(cp));
    user_own_add(devup, project_name_get(pp), change_number);

    /*
     * go through the files in the change and unlock them
     * in the baseline
     */
    for (j = 0;; ++j)
    {
	fstate_src	c_src_data;
	fstate_src	p_src_data;

	c_src_data = change_file_nth(cp, j);
	if (!c_src_data)
	    break;
	p_src_data =
	    project_file_find(pp, c_src_data->file_name, view_path_none);
	if (!p_src_data)
	{
	    /* This is actualy a bug. */
	    continue;
	}
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
	    assert(!p_src_data->about_to_be_created_by ||
		p_src_data->about_to_be_created_by==change_number);
	    assert(!p_src_data->about_to_be_copied_by ||
		p_src_data->about_to_be_copied_by==change_number);
	    if (p_src_data->deleted_by)
	    {
		assert(!p_src_data->about_to_be_copied_by);
		p_src_data->about_to_be_created_by = 0;
	    }
	    else
		project_file_remove(pp, c_src_data->file_name);
	}

	/*
	 * remove the integrate difference time
	 */
	if (c_src_data->idiff_file_fp)
	{
	    fingerprint_type.free(c_src_data->idiff_file_fp);
	    c_src_data->idiff_file_fp = 0;
	}

	/*
	 * remove file test times
	 */
	if (c_src_data->architecture_times)
	{
	    fstate_src_architecture_times_list_type.free
	    (
		c_src_data->architecture_times
	    );
	    c_src_data->architecture_times = 0;
	}
    }

    /*
     * remove the integration directory
     */
    change_verbose(cp, 0, i18n("rm int dir"));
    project_become(pp);
    commit_rmdir_tree_bg(dir);
    project_become_undo();

    /*
     * Make the development directory writable again.
     */
    if
    (
	!change_was_a_branch(cp)
    &&
	project_protect_development_directory_get(pp)
    )
	change_development_directory_chmod_read_write(cp);

    /*
     * write out the data and release the locks
     */
    change_cstate_write(cp);
    user_ustate_write(up);
    user_ustate_write(devup);
    project_pstate_write(pp);
    str_free(dir);
    commit();
    lock_release();

    /*
     * run the notify command
     */
    change_run_integrate_fail_notify_command(cp);

    /*
     * verbose success message
     */
    change_verbose(cp, 0, i18n("integrate fail complete"));
    change_free(cp);
    project_free(pp);
    user_free(up);
    user_free(devup);
    trace(("}\n"));
}


void
integrate_fail(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, integrate_fail_help, },
	{arglex_token_list, integrate_fail_list, },
    };

    trace(("integrate_fail()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), integrate_fail_main);
    trace(("}\n"));
}
