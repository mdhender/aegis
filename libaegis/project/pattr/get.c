/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to get a pattr from a project
 */

#include <pattr.h>
#include <project.h>
#include <project_hist.h>
#include <project/pattr/get.h>


void
project_pattr_get(pp, a)
	project_ty	*pp;
	pattr		a;
{
	string_ty	*s;

	if (!a->description)
	{
		s = project_description_get(pp);
		if (s)
			a->description = str_copy(s);
	}

	if (!a->default_development_directory)
	{
		s = project_default_development_directory_get(pp);
		if (s)
			a->default_development_directory = str_copy(s);
	}

	if (!(a->mask & pattr_developer_may_review_mask))
		a->developer_may_review = project_developer_may_review_get(pp);
	if (!(a->mask & pattr_developer_may_integrate_mask))
		a->developer_may_integrate =
			project_developer_may_integrate_get(pp);
	if (!(a->mask & pattr_reviewer_may_integrate_mask))
		a->reviewer_may_integrate =
			project_reviewer_may_integrate_get(pp);
	if (!(a->mask & pattr_developers_may_create_changes_mask))
		a->developers_may_create_changes =
			project_developers_may_create_changes_get(pp);
	if (!(a->mask & pattr_umask_mask))
		a->umask = project_umask_get(pp);
	if (!(a->mask & pattr_default_test_exemption_mask))
		a->default_test_exemption =
			project_default_test_exemption_get(pp);
	if (!(a->mask & pattr_skip_unlucky_mask))
		a->skip_unlucky =
			project_skip_unlucky_get(pp);
	if (!(a->mask & pattr_compress_database_mask))
		a->compress_database =
			project_compress_database_get(pp);
	if (!(a->mask & pattr_develop_end_action_mask))
		a->develop_end_action =
			project_develop_end_action_get(pp);
	
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
		a->minimum_change_number =
			project_minimum_change_number_get(pp);
	if (!(a->mask & pattr_reuse_change_numbers_mask))
	{
		a->reuse_change_numbers =
			project_reuse_change_numbers_get(pp);
		a->mask |= pattr_reuse_change_numbers_mask;
	}
	if (!a->minimum_branch_number)
		a->minimum_branch_number =
			project_minimum_branch_number_get(pp);
}
