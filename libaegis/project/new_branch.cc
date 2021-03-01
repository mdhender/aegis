//
//	aegis - project change supervisor
//	Copyright (C) 2001-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>

#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/change/branch.h>
#include <libaegis/change.h>
#include <libaegis/cstate.h>
#include <libaegis/os.h>
#include <libaegis/pconf.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>


static string_ty *
branch_description_invent(project_ty *pp)
{
    string_ty	    *s1;
    string_ty	    *s2;
    size_t	    len;
    string_ty	    *result;

    s2 = project_version_short_get(pp);
    pp = pp->trunk_get();
    s1 = project_description_get(pp);
    if (s1->str_length == 0)
    {
	str_free(s2);
	return str_copy(s1);
    }
    for (len = s1->str_length; len; --len)
    {
	unsigned char	c;

	c = s1->str_text[len - 1];
	if (c == '.')
	    continue;
	// C locale
	if (isspace(c))
	    continue;
	break;
    }
    result =
	str_format("%.*s, branch %s.", (int)len, s1->str_text, s2->str_text);
    // do not free s1
    str_free(s2);
    return result;
}


project_ty *
project_new_branch(project_ty *ppp, user_ty::pointer up, long change_number,
    string_ty *topdir, string_ty *reason)
{
    cstate_ty       *cstate_data;
    cstate_history_ty *history_data;
    change::pointer cp;
    size_t	    j;
    pconf_ty        *pconf_data;
    string_ty	    *s;
    project_ty	    *pp;

    //
    // On entry it is assumed that you have
    // a project state lock, to create the branches
    //
    trace(("project_new_branch(ppp = %8.8lX, change_number = %ld, "
           "topdir = %ld, reason = %ld)\n{\n", (long)ppp, change_number,
           (long)topdir, (long)reason));

    //
    // create the new branch.
    // validity of the branch number was checked elsewhere.
    //
    cp = change_alloc(ppp, change_number);
    change_bind_new(cp);
    change_branch_new(cp);
    pp = ppp->bind_branch(cp);

    //
    // The copyright years are not copied from the parent, because
    // they are updated by integrations.  The copyright years thus
    // represent the copyright years for the changes made by the
    // branch.
    //
    // However, when copyright years are calculated for "ael
    // version" the branch's copyright years (actually, all
    // ancestors') are included.
    //

    //
    // Create the change (branch) directory.
    //
    // It is in the same place as the rest of the project, unless
    // otherwise specified.  This makes it easy to collect the whole
    // project tree, branches, info and all, in a simple tar or cpio
    // command for archiving or moving.
    //
    if (!topdir)
    {
	sub_context_ty	*scp;

	topdir =
	    str_format
	    (
		"%s/branch.%ld",
		project_top_path_get(ppp, 0)->str_text,
		magic_zero_decode(change_number)
	    );
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", topdir);
	change_verbose(cp, scp, i18n("development directory \"$filename\""));
	sub_context_delete(scp);
    }
    change_top_path_set(cp, topdir);

    //
    // Create the development directory.
    //
    // In a branch, the development directory contains
    //	    (a) a directory called "baseline" for the branch's baseline
    //	    (b) directories called "delta.*" for integrations, later
    //	    (c) directories called "branch.*" for branches, later
    //
    // A branch's development directory is owned by the project
    // owner, not the creating user.
    //
    // The "info" and "history" directories are only present in the
    // development directory of the trunk.  DO NOT create them here.
    //
    s = str_format("%s/baseline", topdir->str_text);
    project_become(pp);
    os_mkdir(topdir, 02755);
    os_mkdir(s, 02755);
    undo_rmdir_errok(s);
    undo_rmdir_errok(topdir);
    project_become_undo(pp);
    str_free(s);

    //
    // create the change history,
    // and advance the change to 'being developed'
    //
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_new_change;
    if (reason)
	history_data->why = str_copy(reason);
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_develop_begin;
    cstate_data = cp->cstate_get();
    cstate_data->state = cstate_state_being_developed;

    //
    // set the attributes
    //
    cstate_data->description = branch_description_invent(pp);
    cstate_data->brief_description = str_copy(cstate_data->description);
    cstate_data->cause = change_cause_internal_enhancement;
    cstate_data->test_exempt = true;
    cstate_data->test_baseline_exempt = true;
    cstate_data->regression_test_exempt = true;

    //
    // Create symbolic links from the the branch's baseline into the
    // branch's parent's baseline.  This is similar to what happens
    // at develop begin.
    //
    pconf_data = change_pconf_get(cp, 0);
    assert(pconf_data->integration_directory_style);
    work_area_style_ty style = *pconf_data->integration_directory_style;
    if (!style.during_build_only)
	change_create_symlinks_to_baseline(cp, project_user(ppp), style);

    //
    // Clear the time fields.
    //
    change_build_times_clear(cp);

    //
    // many of the branch attributes are inherited from the parent
    //
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
    change_branch_review_begin_notify_command_set
    (
	cp,
	project_review_begin_notify_command_get(ppp)
    );
    change_branch_review_begin_undo_notify_command_set
    (
	cp,
	project_review_begin_undo_notify_command_get(ppp)
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
    change_branch_default_test_regression_exemption_set
    (
	cp,
	project_default_test_regression_exemption_get(ppp)
    );
    change_branch_default_development_directory_set
    (
	cp,
	project_default_development_directory_get(ppp)
    );
    change_branch_compress_database_set
    (
	cp,
	project_compress_database_get(ppp)
    );
    change_branch_protect_development_directory_set
    (
	cp,
	project_protect_development_directory_get(ppp)
    );

    //
    // staff lists are inherited from the parent
    //
    for (j = 0;; ++j)
    {
	s = project_administrator_nth(ppp, j);
	if (!s)
	    break;
	change_branch_administrator_add(cp, s);
    }
    for (j = 0;; ++j)
    {
	s = project_developer_nth(ppp, j);
	if (!s)
	    break;
	change_branch_developer_add(cp, s);
    }
    for (j = 0;; ++j)
    {
	s = project_reviewer_nth(ppp, j);
	if (!s)
	    break;
	change_branch_reviewer_add(cp, s);
    }
    for (j = 0;; ++j)
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
    change_branch_skip_unlucky_set(cp, project_skip_unlucky_get(ppp));
    change_branch_develop_end_action_set
    (
	cp,
	project_develop_end_action_get(ppp)
    );

    //
    // architecture is inherited from parent
    //
    assert(pconf_data->architecture);
    change_architecture_clear(cp);
    for (j = 0; j < pconf_data->architecture->length; ++j)
    {
	change_architecture_add(cp, pconf_data->architecture->list[j]->name);
    }

    //
    // Add the change to the list of existing changes
    //
    project_change_append(ppp, change_number, 1);
    trace(("return %8.8lX;\n", (long)pp));
    trace(("}\n"));
    return pp;
}
