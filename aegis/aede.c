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
 * MANIFEST: functions to implement develop end
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <aede.h>
#include <ael/change/by_state.h>
#include <arglex2.h>
#include <change/branch.h>
#include <change/develop_direct/read_only.h>
#include <change/file.h>
#include <col.h>
#include <commit.h>
#include <common.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <progname.h>
#include <os.h>
#include <project.h>
#include <project/active.h>
#include <project/file.h>
#include <project/history.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void
develop_end_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -Develop_End [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Develop_End -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Develop_End -Help\n", progname);
    quit(1);
}


static void
develop_end_help(void)
{
    help("aede", develop_end_usage);
}


static void
develop_end_list(void)
{
    string_ty	    *project_name;

    trace(("develop_end_list()\n{\n"));
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
		option_needs_name(arglex_token_project, develop_end_usage);
	    /* fall through... */

	case arglex_token_string:
	    if (project_name)
	    {
		duplicate_option_by_name
		(
		    arglex_token_project,
		    develop_end_usage
		);
	    }
	    project_name = str_from_c(arglex_value.alv_string);
	    break;
	}
	arglex();
    }
    list_changes_in_state_mask(project_name, 1 << cstate_state_being_developed);
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


static void
develop_end_main(void)
{
    sub_context_ty  *scp;
    string_ty	    *dd;
    cstate	    cstate_data;
    int		    j;
    cstate_history  history_data;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    user_ty	    *up;
    user_ty	    *up_admin;
    int		    diff_whine;
    int		    errs;
    time_t	    youngest;
    string_ty	    *youngest_name;
    int		    config_seen;
    int		    is_a_branch;

    trace(("develop_end_main()\n{\n"));
    arglex();
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
		option_needs_number(arglex_token_change, develop_end_usage);
	    /* fall through... */

	case arglex_token_number:
	    if (change_number)
	    {
		duplicate_option_by_name
		(
		    arglex_token_change,
		    develop_end_usage
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
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, develop_end_usage);
	    /* fall through... */

	case arglex_token_string:
	    if (project_name)
	    {
		duplicate_option_by_name
		(
		    arglex_token_project,
		    develop_end_usage
		);
	    }
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(develop_end_usage);
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
    up_admin = 0;

    /*
     * locate change data
     */
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    /*
     * Project administrators are allowed to end the development of
     * a branch, no matter who created it.
     */
    if
    (
	change_is_a_branch(cp)
    &&
	!str_equal(change_developer_name(cp), user_name(up))
    &&
	project_administrator_query(pp, user_name(up))
    )
    {
	up_admin = up;
	up = user_symbolic(pp, change_developer_name(cp));
    }

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

    /*
     * It is an error if the change is not in the being_developed state.
     * It is an error if the change is not assigned to the current user.
     * It is an error if the change has no current diff.
     * It is an error if the change has no current build.
     * It is an error if the change has no current test pass.
     * It is an error if the change has no current baseline test pass.
     * It is an error if the change has no new test associated with it.
     */
    if (cstate_data->state != cstate_state_being_developed)
	change_fatal(cp, 0, i18n("bad de state"));
    if (!str_equal(change_developer_name(cp), user_name(up)))
	change_fatal(cp, 0, i18n("not developer"));
    if (!change_file_nth(cp, (size_t)0))
	change_fatal(cp, 0, i18n("no files"));
    errs = 0;
    diff_whine = 0;
    config_seen = 0;

    /*
     * It is an error if change change attributes mention
     * architectures not in the project.
     */
    change_check_architectures(cp);

    /*
     * It is an error if any files in the change file table have
     * been modified since the last build.
     */
    youngest = 0;
    youngest_name = 0;
    is_a_branch = change_is_a_branch(cp);
    for (j = 0;; ++j)
    {
	fstate_src	c_src_data;
	fstate_src	p_src_data;
	string_ty	*path;
	string_ty	*path_d;
	int		same;
	int		same_d;
	int		file_required;
	int		diff_file_required;

	c_src_data = change_file_nth(cp, j);
	if (!c_src_data)
	    break;
	trace(("file_name = \"%s\"\n", c_src_data->file_name->str_text));

	file_required = 1;
	diff_file_required = 1;
	switch (c_src_data->action)
	{
	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
	    break;

	case file_action_remove:
	    file_required = 0;
	    if (is_a_branch)
	    {
		fstate_src      src;

		src =
		    project_file_find
		    (
			pp,
			c_src_data->file_name,
			view_path_extreme
		    );
		if (!src)
		{
		    diff_file_required = 0;
		}
	    }

	    /*
	     * The removed half of a move is not differenced.
	     */
	    if (c_src_data->move && change_file_find(cp, c_src_data->move))
		diff_file_required = 0;
	    break;

	case file_action_transparent:
	    /*
	     * Don't check anything for branches (the file is going).
	     * For changes, make sure it's the same as the ancestor.
	     */
	    diff_file_required = 0;
	    file_required = 0;
	    if (!is_a_branch)
	    {
		assert(pp->parent);
		if (pp->parent)
		{
		    fstate_src      pp_src;

		    pp_src =
			project_file_find
			(
			    pp->parent,
			    c_src_data->file_name,
			    view_path_extreme
			);
		    if (pp_src)
		    {
			string_ty       *blf;
			int             different;

			path = change_file_path(cp, c_src_data->file_name);
			blf =
			    project_file_path
			    (
				pp->parent,
				c_src_data->file_name
			    );
			assert(blf);
			user_become(up);
			different = files_are_different(path, blf);
			user_become_undo();
			str_free(blf);
			str_free(path);
			if (different)
			{
		    	    scp = sub_context_new();
			    sub_var_set_string
			    (
				scp,
				"File_Name",
				c_src_data->file_name
			    );
			    change_error(cp, scp, i18n("$filename altered"));
			    sub_context_delete(scp);
			    ++errs;
			}
		    }
		}
	    }
	    break;
	}
	if (c_src_data->usage == file_usage_build)
	{
	    file_required = 0;
	    diff_file_required = 0;
	}

	/*
	 * the config file in a change
	 * implies additional tests
	 */
	if (change_file_is_config(cp, c_src_data->file_name))
	    config_seen++;

	/*
	 * make sure the file exists and has not altered
	 */
	path = change_file_path(cp, c_src_data->file_name);
	if (file_required)
	{
	    if (!c_src_data->file_fp)
		c_src_data->file_fp = fingerprint_type.alloc();
	    assert(c_src_data->file_fp->youngest>=0);
	    assert(c_src_data->file_fp->oldest>=0);
	    user_become(up);
	    same = change_fingerprint_same(c_src_data->file_fp, path, 0);
	    user_become_undo();
	    assert(c_src_data->file_fp->youngest>0);
	    assert(c_src_data->file_fp->oldest>0);
	    trace(("same = %d\n", same));

	    if (!c_src_data->file_fp || !c_src_data->file_fp->youngest)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", c_src_data->file_name);
		change_error(cp, scp, i18n("$filename not found"));
		sub_context_delete(scp);
		str_free(path);
		errs++;
		continue;
	    }

	    if (c_src_data->file_fp->oldest > youngest)
	    {
		youngest = c_src_data->file_fp->oldest;
		youngest_name = c_src_data->file_name;
	    }
	}
	else
	    same = 1;

	/*
	 * make sure the filename conforms to length limits
	 *
	 * Scenario: user copies "config", alters filename
	 * constraints, creates file, uncopies "config".
	 * Reviewer will not necessarily notice, especially when
	 * expecting aegis to notice for him.
	 */
	if (!is_a_branch && file_required)
	{
	    string_ty	    *e;

	    e = change_filename_check(cp, c_src_data->file_name);
	    if (e)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "MeSsaGe", e);
		str_free(e);
		change_error(cp, scp, i18n("$message"));
		sub_context_delete(scp);
		++errs;
	    }
	}

	/*
	 * make sure the difference file exists and has not been altered
	 */
	if (diff_file_required)
	{
	    path_d = str_format("%S,D", path);
	    if (!c_src_data->diff_file_fp)
		c_src_data->diff_file_fp = fingerprint_type.alloc();
	    user_become(up);
	    same_d =
		change_fingerprint_same(c_src_data->diff_file_fp, path_d, 0);
	    user_become_undo();
	    trace(("same_d = %d\n", same_d));
	    str_free(path_d);

	    if
	    (
		!c_src_data->diff_file_fp
	    ||
		!c_src_data->diff_file_fp->youngest
	    )
	    {
		scp = sub_context_new();
		sub_var_set_format
		(
		    scp,
		    "File_Name",
		    "%S,D",
		    c_src_data->file_name
		);
		change_error(cp, scp, i18n("$filename not found"));
		sub_context_delete(scp);
		errs++;
	    }
	}
	else
	    same_d = 1;
	str_free(path);

	/*
	 * check that a difference has been done,
	 * and that no files have been modified since.
	 */
	if (!diff_whine)
	{
	    if (file_required && !same && c_src_data->file_fp->oldest)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", c_src_data->file_name);
		change_error(cp, scp, i18n("$filename changed after diff"));
		sub_context_delete(scp);
		diff_whine++;
		errs++;
	    }
	    else if
	    (
		diff_file_required
	    &&
		!same_d
	    &&
		c_src_data->diff_file_fp->youngest
	    )
	    {
		scp = sub_context_new();
		sub_var_set_format
		(
		    scp,
		    "File_Name",
		    "%S,D",
		    c_src_data->file_name
		);
		change_error(cp, scp, i18n("$filename changed after diff"));
		sub_context_delete(scp);
		diff_whine++;
		errs++;
	    }
	    else if
	    (
		(file_required && !same)
	    ||
		(diff_file_required && !same_d)
	    )
	    {
		change_error(cp, 0, i18n("diff required"));
		diff_whine++;
		errs++;
	    }
	}

	/*
	 * For each change file that is acting on a project file
	 * from a deeper level than the immediate parent
	 * project, the file needs to be added to the immediate
	 * parent project.
	 *
	 * This is where the about_to_be_copied_by attribute comes from.
	 * Nothing is done for files being created.
	 */
	project_file_shallow(pp, c_src_data->file_name, change_number);

	/*
	 * Find the project after ``shallowing'' it, as this
	 * gives the project file on the immediate branch,
	 * rather than deeper down the family tree.
	 */
	p_src_data =
	    project_file_find(pp, c_src_data->file_name, view_path_none);

	/*
	 * It is an error if any files in the change
	 * file table have different edit numbers to the
	 * baseline file table edit numbers.
	 */
	switch (c_src_data->action)
	{
	case file_action_remove:
	    /*
	     * file being removed
	     */
	    if
	    (
		!is_a_branch
	    &&
		!project_file_find(pp, c_src_data->file_name, view_path_extreme)
	    )
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", c_src_data->file_name);
		change_error(cp, scp, i18n("no $filename in baseline"));
		sub_context_delete(scp);
		errs++;
	    }

	    /*
	     * Make sure the file exists in the project.
	     *
	     * This can happen for aede on branches which
	     * have had a new file created and then deleted;
	     * thus the file will not exist in the branch's
	     * project.
	     */
	    if (!p_src_data)
	    {
		p_src_data = project_file_new(pp, c_src_data->file_name);
		p_src_data->usage = c_src_data->usage;
		p_src_data->about_to_be_created_by = change_number;
		assert(c_src_data->edit||c_src_data->edit_origin);
		p_src_data->edit =
		    history_version_copy
		    (
			c_src_data->edit
		    ?
			c_src_data->edit
		    :
			c_src_data->edit_origin
		    );
		p_src_data->edit_origin =
		    history_version_copy(p_src_data->edit);
		break;
	    }

	    /*
	     * Make sure the edit numbers match.  If it
	     * matches any of the ancestral edits, it does
	     * not require a merge.
	     */
	    if (file_required && !change_file_up_to_date(pp, c_src_data))
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", c_src_data->file_name);
		if (is_a_branch)
		{
		    change_error
		    (
			cp,
			scp,
			i18n("baseline $filename changed, merge in new change")
		    );
		}
		else
		{
		    change_error(cp, scp, i18n("baseline $filename changed"));
		}
		sub_context_delete(scp);
		errs++;
	    }

	    /*
	     * make sure we can lock the file
	     */
	    if (p_src_data->locked_by)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", c_src_data->file_name);
		sub_var_set_long(scp, "Number", p_src_data->locked_by);
		change_error
		(
		    cp,
		    scp,
		    i18n("file \"$filename\" locked for change $number")
		);
		sub_context_delete(scp);
		errs++;
	    }
	    else
		p_src_data->locked_by = change_number;
	    break;

	case file_action_transparent:
	    /*
	     * Do absolutely nothing for transparent branch files.
	     */
	    if (change_was_a_branch(cp))
		break;
	    /* fall through... */

	case file_action_modify:
	    /*
	     * file being modified
	     */
	    if
	    (
		!is_a_branch
	    &&
		!project_file_find(pp, c_src_data->file_name, view_path_extreme)
	    )
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", c_src_data->file_name);
		change_error(cp, scp, i18n("no $filename in baseline"));
		sub_context_delete(scp);
		errs++;
		continue;
	    }

	    /*
	     * Make sure the file exists in the project.
	     *
	     * This can happen for aede on branches which
	     * have had a new file created and then modified;
	     * thus the file will not exist in the branch's
	     * project.
	     */
	    if (!p_src_data)
	    {
		p_src_data = project_file_new(pp, c_src_data->file_name);
		p_src_data->usage = c_src_data->usage;
		p_src_data->action = file_action_transparent;
		p_src_data->about_to_be_created_by = change_number;
		assert(c_src_data->edit || c_src_data->edit_origin);
		p_src_data->edit =
		    history_version_copy
		    (
			c_src_data->edit
		    ?
			c_src_data->edit
		    :
			c_src_data->edit_origin
		    );
		p_src_data->edit_origin =
		    history_version_copy(p_src_data->edit);
		break;
	    }

	    /*
	     * Make sure the edit numbers match.  If it
	     * matches any of the ancestral edits, it does
	     * not require a merge.
	     */
	    if (!change_file_up_to_date(pp, c_src_data))
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", c_src_data->file_name);
		change_error(cp, scp, i18n("baseline $filename changed"));
		sub_context_delete(scp);
		errs++;
	    }

	    /*
	     * make sure we can lock the file
	     */
	    if (p_src_data->locked_by)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", c_src_data->file_name);
		sub_var_set_long(scp, "Number", p_src_data->locked_by);
		change_error
		(
		    cp,
		    scp,
		    i18n("file \"$filename\" locked for change $number")
		);
		sub_context_delete(scp);
		errs++;
	    }
	    else
		p_src_data->locked_by = change_number;
	    break;

	case file_action_create:
	    /*
	     * file being created
	     */
	    if (p_src_data)
	    {
		if (p_src_data->about_to_be_created_by)
		{
		    scp = sub_context_new();
		    sub_var_set_string(scp, "File_Name", c_src_data->file_name);
		    sub_var_set_long
		    (
			scp,
			"Number",
			c_src_data->about_to_be_created_by
		    );
		    change_error
		    (
			cp,
			scp,
			i18n("file \"$filename\" locked for change $number")
		    );
		    sub_context_delete(scp);
		    ++errs;
		}
		else if (!p_src_data->deleted_by)
		{
		    scp = sub_context_new();
		    sub_var_set_string(scp, "File_Name", c_src_data->file_name);
		    change_error(cp, scp, i18n("$filename in baseline"));
		    sub_context_delete(scp);
		    ++errs;
		}
	    }
	    else
	    {
		/*
		 * add a new entry to the pstate src list,
		 * and mark it as "about to be created".
		 */
		p_src_data = project_file_new(pp, c_src_data->file_name);
		p_src_data->usage = c_src_data->usage;
		p_src_data->action = file_action_transparent;
		p_src_data->about_to_be_created_by = change_number;
	    }
	    p_src_data->locked_by = change_number;
	    break;

	case file_action_insulate:
	    /*
	     * There should be no insulation files in the
	     * change at develop end time.  This is because
	     * if the change is still being insulated from
	     * the baseline, something could easily be
	     * overlooked.
	     */
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", c_src_data->file_name);
	    change_error(cp, scp, i18n("$filename is insulation"));
	    sub_context_delete(scp);
	    errs++;
	    break;
	}
    }

    /*
     * It is an error if this change is a branch, and there are any
     * changes outstanding on the branch.
     */
    if (is_a_branch)
	project_active_check_branch(cp, 0);

    /*
     * if the config file changes,
     * make sure the project file names still conform
     */
    if (config_seen)
    {
	for (j = 0;; ++j)
	{
	    fstate_src	    p_src_data;
	    string_ty	    *e;

	    p_src_data = project_file_nth(pp, j, view_path_extreme);
	    if (!p_src_data)
		break;
	    if (change_file_find(cp, p_src_data->file_name))
		continue;

	    e = change_filename_check(cp, p_src_data->file_name);
	    if (e)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "Message", e);
		str_free(e);
		change_error(cp, scp, i18n("project $message"));
		sub_context_delete(scp);
		++errs;
	    }
	}
    }

    /*
     * verify that the youngest file is older than the
     * build, test, test -bl and test -reg times
     */
    if (!cstate_data->build_time || youngest >= cstate_data->build_time)
    {
	if (youngest_name && cstate_data->build_time)
	{
	    scp = sub_context_new();
	    sub_var_set_charstar
	    (
		scp,
		"Outstanding",
		change_outstanding_builds(cp, youngest)
	    );
	    sub_var_optional(scp, "Outstanding");
	    sub_var_set_string(scp, "File_Name", youngest_name);
	    change_error(cp, scp, i18n("$filename changed after build"));
	    sub_context_delete(scp);
	}
	else
	{
	    scp = sub_context_new();
	    sub_var_set_charstar
	    (
		scp,
		"Outstanding",
		change_outstanding_builds(cp, youngest)
	    );
	    sub_var_optional(scp, "Outstanding");
	    change_error(cp, scp, i18n("build required"));
	    sub_context_delete(scp);
	}
	++errs;
    }
    if
    (
	!cstate_data->test_exempt
    &&
	(!cstate_data->test_time || youngest >= cstate_data->test_time)
    )
    {
	scp = sub_context_new();
	sub_var_set_charstar
	(
	    scp,
	    "Outstanding",
	    change_outstanding_tests(cp, youngest)
	);
	sub_var_optional(scp, "Outstanding");
	change_error(cp, scp, i18n("test required"));
	sub_context_delete(scp);
	++errs;
    }
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
	scp = sub_context_new();
	sub_var_set_charstar
	(
	    scp,
	    "Outstanding",
	    change_outstanding_tests_baseline(cp, youngest)
	);
	sub_var_optional(scp, "Outstanding");
	change_error(cp, scp, i18n("test -bl required"));
	sub_context_delete(scp);
	++errs;
    }
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
	scp = sub_context_new();
	sub_var_set_charstar
	(
	    scp,
	    "Outstanding",
	    change_outstanding_tests_regression(cp, youngest)
	);
	sub_var_optional(scp, "Outstanding");
	change_error(cp, scp, i18n("test -reg required"));
	sub_context_delete(scp);
	++errs;
    }

    /*
     * if there was any problem,
     * stay in 'being developed' state.
     */
    if (errs)
    {
	scp = sub_context_new();
	sub_var_set_long(scp, "Number", errs);
	sub_var_optional(scp, "Number");
	change_fatal(cp, scp, i18n("develop end fail"));
	sub_context_delete(scp);
    }
    dd = str_copy(change_development_directory_get(cp, 1));
    str_free(dd);

    /*
     * add to history for state change
     * Advance the change to the being-reviewed state.
     */
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_develop_end;
    cstate_data->state = cstate_state_being_reviewed;
    switch (project_develop_end_action_get(pp))
    {
    case cstate_branch_develop_end_action_goto_being_reviewed:
	history_data->what = cstate_history_what_develop_end;
	cstate_data->state = cstate_state_being_reviewed;
	break;

    case cstate_branch_develop_end_action_goto_awaiting_review:
	history_data->what = cstate_history_what_develop_end_2ar;
	cstate_data->state = cstate_state_awaiting_review;
	break;

    case cstate_branch_develop_end_action_goto_awaiting_integration:
	history_data->what = cstate_history_what_develop_end_2ai;
	cstate_data->state = cstate_state_awaiting_integration;
	break;
    }
    if (up_admin)
    {
	history_data->why =
	    str_format("Forced by administrator \"%S\".", user_name(up_admin));
    }

    /*
     * It is tempting to call
     *
     change_build_times_clear(cp);
     *
     * at this point, but we don't.  This is so that in the event
     * of an aedeu (to get out of the way of another change, maybe)
     * a full build-and-test cycle is only required of they actually
     * edit something.
     */

    /*
     * Remove the change from the list of assigned changes in the user
     * change table (in the user row).
     */
    user_own_remove(up, project_name_get(pp), change_number);

    /*
     * Make the development directory read only.
     */
    if
    (
	!change_was_a_branch(cp)
    &&
	project_protect_development_directory_get(pp)
    )
	change_development_directory_chmod_read_only(cp);

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
    change_verbose(cp, 0, i18n("development completed"));
    change_free(cp);
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}


void
develop_end(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, develop_end_help, },
	{arglex_token_list, develop_end_list, },
    };

    trace(("develop_end()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), develop_end_main);
    trace(("}\n"));
}
