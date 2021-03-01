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
#include <libaegis/project/pattr/set.h>
#include <libaegis/sub.h>


void
project_pattr_set(project *pp, pattr_ty *pattr_data)
{
    if (pattr_data->description)
    {
        string_ty *s = str_snip(pattr_data->description);
        project_description_set(pp, s);
        str_free(s);
    }

    if (pattr_data->mask & pattr_developer_may_review_mask)
        project_developer_may_review_set(pp, pattr_data->developer_may_review);
    if (pattr_data->mask & pattr_developer_may_integrate_mask)
    {
        project_developer_may_integrate_set
        (
            pp,
            pattr_data->developer_may_integrate
        );
    }
    if (pattr_data->mask & pattr_reviewer_may_integrate_mask)
    {
        project_reviewer_may_integrate_set
        (
            pp,
            pattr_data->reviewer_may_integrate
        );
    }
    if (pattr_data->mask & pattr_developers_may_create_changes_mask)
    {
        project_developers_may_create_changes_set
        (
            pp,
            pattr_data->developers_may_create_changes
        );
    }

    if (pattr_data->mask & pattr_umask_mask)
        project_umask_set(pp, pattr_data->umask);

    if (pattr_data->mask & pattr_default_test_exemption_mask)
    {
        project_default_test_exemption_set
        (
            pp,
            pattr_data->default_test_exemption
        );
    }
    if (pattr_data->mask & pattr_default_test_regression_exemption_mask)
    {
        project_default_test_regression_exemption_set
        (
            pp,
            pattr_data->default_test_regression_exemption
        );
    }
    if (pattr_data->mask & pattr_skip_unlucky_mask)
        project_skip_unlucky_set(pp, pattr_data->skip_unlucky);
    if (pattr_data->mask & pattr_compress_database_mask)
        project_compress_database_set(pp, pattr_data->compress_database);
    if (pattr_data->mask & pattr_develop_end_action_mask)
        project_develop_end_action_set(pp, pattr_data->develop_end_action);

    //
    // Make sure that developers_may_review and
    // develop_end_action don't contradict each other.
    //
    // At this point, we quietly change the action, but
    // maybe we should emit a fatal error message?
    //
    if
    (
        !project_developer_may_review_get(pp)
    &&
        project_develop_end_action_get(pp) ==
            pattr_develop_end_action_goto_awaiting_integration
    )
    {
        project_develop_end_action_set
        (
            pp,
            pattr_develop_end_action_goto_being_reviewed
        );
    }

    if (pattr_data->forced_develop_begin_notify_command)
    {
        project_forced_develop_begin_notify_command_set
        (
            pp,
            pattr_data->forced_develop_begin_notify_command
        );
    }

    if (pattr_data->develop_end_notify_command)
    {
        project_develop_end_notify_command_set
        (
            pp,
            pattr_data->develop_end_notify_command
        );
    }

    if (pattr_data->develop_end_undo_notify_command)
    {
        project_develop_end_undo_notify_command_set
        (
            pp,
            pattr_data->develop_end_undo_notify_command
        );
    }

    if (pattr_data->review_begin_notify_command)
    {
        project_review_begin_notify_command_set
        (
            pp,
            pattr_data->review_begin_notify_command
        );
    }
    if (pattr_data->review_begin_undo_notify_command)
    {
        project_review_begin_undo_notify_command_set
        (
            pp,
            pattr_data->review_begin_undo_notify_command
        );
    }

    if (pattr_data->review_pass_notify_command)
    {
        project_review_pass_notify_command_set
        (
            pp,
            pattr_data->review_pass_notify_command
        );
    }

    if (pattr_data->review_pass_undo_notify_command)
    {
        project_review_pass_undo_notify_command_set
        (
            pp,
            pattr_data->review_pass_undo_notify_command
        );
    }

    if (pattr_data->review_fail_notify_command)
    {
        project_review_fail_notify_command_set
        (
            pp,
            pattr_data->review_fail_notify_command
        );
    }

    if (pattr_data->integrate_pass_notify_command)
    {
        project_integrate_pass_notify_command_set
        (
            pp,
            pattr_data->integrate_pass_notify_command
        );
    }

    if (pattr_data->integrate_fail_notify_command)
    {
            project_integrate_fail_notify_command_set
            (
                    pp,
                    pattr_data->integrate_fail_notify_command
            );
    }

    if (pattr_data->default_development_directory)
    {
        string_ty       *s;

        s = pattr_data->default_development_directory;
        if (!s->str_length)
            s = 0;
        else
        {
            if (s->str_text[0] != '/')
                fatal_intl(0, i18n("bad pa, rel def dev dir"));
        }
        project_default_development_directory_set(pp, s);
    }

    if (pattr_data->mask & pattr_minimum_change_number_mask)
    {
        project_minimum_change_number_set
        (
            pp,
            pattr_data->minimum_change_number
        );
    }
    if (pattr_data->mask & pattr_reuse_change_numbers_mask)
    {
        project_reuse_change_numbers_set
        (
            pp,
            pattr_data->reuse_change_numbers
        );
    }
    if (pattr_data->mask & pattr_minimum_branch_number_mask)
    {
        project_minimum_branch_number_set
        (
            pp,
            pattr_data->minimum_branch_number
        );
    }
    if (pattr_data->mask & pattr_protect_development_directory_mask)
    {
        project_protect_development_directory_set
        (
            pp,
            pattr_data->protect_development_directory
        );
    }
}


// vim: set ts=8 sw=4 et :
