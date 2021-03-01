//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2006, 2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <libaegis/pattr.fmtgen.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/project/pattr/get.h>


void
project_pattr_get(project *pp, pattr_ty *a)
{
    string_ty       *s;

    if (!a->description)
        a->description = project_description_get(pp).get_ref_copy();

    if (!a->default_development_directory)
    {
        s = project_default_development_directory_get(pp);
        if (s)
            a->default_development_directory = str_copy(s);
    }

    if (!(a->mask & pattr_developer_may_review_mask))
    {
        a->developer_may_review = project_developer_may_review_get(pp);
    }
    if (!(a->mask & pattr_developer_may_integrate_mask))
    {
        a->developer_may_integrate = project_developer_may_integrate_get(pp);
    }
    if (!(a->mask & pattr_reviewer_may_integrate_mask))
    {
        a->reviewer_may_integrate = project_reviewer_may_integrate_get(pp);
    }
    if (!(a->mask & pattr_developers_may_create_changes_mask))
    {
        a->developers_may_create_changes =
            project_developers_may_create_changes_get(pp);
    }
    if (!(a->mask & pattr_umask_mask))
    {
        a->umask = project_umask_get(pp);
    }
    if (!(a->mask & pattr_default_test_exemption_mask))
    {
        a->default_test_exemption = project_default_test_exemption_get(pp);
    }
    if (!(a->mask & pattr_default_test_regression_exemption_mask))
    {
        a->default_test_regression_exemption =
            project_default_test_regression_exemption_get(pp);
        a->mask |= pattr_default_test_regression_exemption_mask;
    }
    if (!(a->mask & pattr_skip_unlucky_mask))
    {
        a->skip_unlucky = project_skip_unlucky_get(pp);
    }
    if (!(a->mask & pattr_compress_database_mask))
    {
        a->compress_database = project_compress_database_get(pp);
    }
    if (!(a->mask & pattr_develop_end_action_mask))
    {
        a->develop_end_action =
            (pattr_develop_end_action_ty)project_develop_end_action_get(pp);
    }

    if (!a->forced_develop_begin_notify_command)
    {
        s = project_forced_develop_begin_notify_command_get(pp);
        if (s)
            a->forced_develop_begin_notify_command = str_copy(s);
    }

    if (!a->develop_end_notify_command)
    {
        s = project_develop_end_notify_command_get(pp);
        if (s)
            a->develop_end_notify_command = str_copy(s);
    }

    if (!a->develop_end_undo_notify_command)
    {
        s = project_develop_end_undo_notify_command_get(pp);
        if (s)
            a->develop_end_undo_notify_command = str_copy(s);
    }

    if (!a->review_begin_notify_command)
    {
        s = project_review_begin_notify_command_get(pp);
        if (s)
            a->review_begin_notify_command = str_copy(s);
    }
    if (!a->review_begin_undo_notify_command)
    {
        s = project_review_begin_undo_notify_command_get(pp);
        if (s)
            a->review_begin_undo_notify_command = str_copy(s);
    }

    if (!a->review_pass_notify_command)
    {
        s = project_review_pass_notify_command_get(pp);
        if (s)
            a->review_pass_notify_command = str_copy(s);
    }

    if (!a->review_pass_undo_notify_command)
    {
        s = project_review_pass_undo_notify_command_get(pp);
        if (s)
            a->review_pass_undo_notify_command = str_copy(s);
    }

    if (!a->review_fail_notify_command)
    {
        s = project_review_fail_notify_command_get(pp);
        if (s)
            a->review_fail_notify_command = str_copy(s);
    }

    if (!a->integrate_pass_notify_command)
    {
        s = project_integrate_pass_notify_command_get(pp);
        if (s)
            a->integrate_pass_notify_command = str_copy(s);
    }

    if (!a->integrate_fail_notify_command)
    {
        s = project_integrate_fail_notify_command_get(pp);
        if (s)
            a->integrate_fail_notify_command = str_copy(s);
    }

    if (!a->minimum_change_number)
    {
        a->minimum_change_number = project_minimum_change_number_get(pp);
    }
    if (!(a->mask & pattr_reuse_change_numbers_mask))
    {
        a->reuse_change_numbers = project_reuse_change_numbers_get(pp);
        a->mask |= pattr_reuse_change_numbers_mask;
    }
    if (!a->minimum_branch_number)
    {
        a->minimum_branch_number = project_minimum_branch_number_get(pp);
    }
    if (!(a->mask & pattr_protect_development_directory_mask))
    {
        a->protect_development_directory =
            project_protect_development_directory_get(pp);
    }
}


// vim: set ts=8 sw=4 et :
