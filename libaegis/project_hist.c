/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995-1998, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate project history
 */

#include <change/branch.h>
#include <error.h>
#include <project_hist.h>
#include <trace.h>
#include <str_list.h>
#include <zero.h>


int
project_history_delta_validate(pp, delta_number)
	project_ty	*pp;
	long		delta_number;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	return change_history_delta_validate(cp, delta_number);
}


time_t
project_history_delta_to_timestamp(pp, when)
	project_ty	*pp;
	time_t		when;
{
	return change_history_delta_to_timestamp(pp, when);
}


long
project_history_delta_latest(pp)
	project_ty	*pp;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	return change_history_delta_latest(cp);
}


long
project_history_delta_by_name(pp, delta_name, errok)
	project_ty	*pp;
	string_ty	*delta_name;
	int		errok;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	return change_history_delta_by_name(cp, delta_name, errok);
}


void
project_history_delta_name_delete(pp, delta_name)
	project_ty	*pp;
	string_ty	*delta_name;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	change_history_delta_name_delete(cp, delta_name);
}


void
project_history_delta_name_add(pp, delta_number, delta_name)
	project_ty	*pp;
	long		delta_number;
	string_ty	*delta_name;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	change_history_delta_name_add(cp, delta_number, delta_name);
}


long
project_current_integration_get(pp)
	project_ty	*pp;
{
	change_ty	*cp;
	long		result;

	trace(("project_current_integration_get(pp = %8.8lX)\n{\n"/*}*/, (long)pp));
	cp = project_change_get(pp);
	result = change_current_integration_get(cp);
	trace(("return %ld;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


void
project_current_integration_set(pp, change_number)
	project_ty	*pp;
	long		change_number;
{
	change_ty	*cp;

	trace(("project_current_integration_set(pp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)pp, change_number));
	assert(change_number >= 0 || change_number == MAGIC_ZERO);
	cp = project_change_get(pp);
	change_current_integration_set(cp, change_number);
	trace((/*{*/"}\n"));
}


long
project_next_delta_number(pp)
	project_ty	*pp;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	return change_branch_next_delta_number(cp);
}


void
project_copyright_years_merge(pp, cp2)
	project_ty	*pp;
	change_ty	*cp2;
{
	change_ty	*cp1;

	cp1 = project_change_get(pp);
	change_copyright_years_merge(cp1, cp2);
}


void
project_copyright_years_get(pp, ary, ary_len_max, ary_len)
	project_ty	*pp;
	int		*ary;
	int		ary_len_max;
	int		*ary_len;
{
	change_ty	*cp;

	/*
	 * when we have the real thing, will need to get the years
	 * from parent branches back to the trunk
	 */
	cp = project_change_get(pp);
	change_copyright_years_get(cp, ary, ary_len_max, ary_len);
}


void
project_copyright_year_append(pp, yyyy)
	project_ty	*pp;
	int		yyyy;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	change_copyright_year_append(cp, yyyy);
}


void
project_history_new(pp, delta_number, change_number)
	project_ty	*pp;
	long		delta_number;
	long		change_number;
{
	change_ty	*cp;

	trace(("project_history_new(pp = %8.8lX, delta_number = %ld, change_number = %ld)\n{\n"/*}*/,
		(long)pp, delta_number, change_number));
	cp = project_change_get(pp);
	change_branch_history_new(cp, delta_number, change_number);
	trace((/*{*/"}\n"));
}


int
project_history_nth(pp, n, cnp, dnp, name)
	project_ty	*pp;
	long		n;
	long		*cnp;
	long		*dnp;
	string_list_ty		*name;
{
	change_ty	*cp;
	int		result;

	trace(("project_history_nth(pp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)pp, n));
	cp = project_change_get(pp);
	result = change_branch_history_nth(cp, n, cnp, dnp, name);
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


int
project_administrator_query(pp, user_name)
	project_ty	*pp;
	string_ty	*user_name;
{
	change_ty	*cp;
	int		result;

	trace(("project_administrator_query(pp = %8.8lX, user_name = \"%s\")\n{\n"/*}*/,
		(long)pp, user_name->str_text));
	cp = project_change_get(pp);
	result = change_branch_administrator_query(cp, user_name);
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


void
project_administrator_add(pp, user_name)
	project_ty	*pp;
	string_ty	*user_name;
{
	change_ty	*cp;

	trace(("project_administrator_add(pp = %8.8lX, user_name = \"%s\")\n{\n"/*}*/,
		(long)pp, user_name->str_text));
	cp = project_change_get(pp);
	change_branch_administrator_add(cp, user_name);
	trace((/*{*/"}\n"));
}


void
project_administrator_remove(pp, user_name)
	project_ty	*pp;
	string_ty	*user_name;
{
	change_ty	*cp;

	trace(("project_administrator_remove(pp = %8.8lX, user_name = \"%s\")\n{\n"/*}*/,
		(long)pp, user_name->str_text));
	cp = project_change_get(pp);
	change_branch_administrator_remove(cp, user_name);
	trace((/*{*/"}\n"));
}


string_ty *
project_administrator_nth(pp, n)
	project_ty	*pp;
	long		n;
{
	change_ty	*cp;
	string_ty	*result;

	trace(("project_administrator_nth(pp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)pp, n));
	cp = project_change_get(pp);
	result = change_branch_administrator_nth(cp, n);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


int
project_developer_query(pp, user_name)
	project_ty	*pp;
	string_ty	*user_name;
{
	change_ty	*cp;
	int		result;

	trace(("project_developer_query(pp = %8.8lX, user_name = \"%s\")\n{\n"/*}*/,
		(long)pp, user_name->str_text));
	cp = project_change_get(pp);
	result = change_branch_developer_query(cp, user_name);
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


void
project_developer_add(pp, user_name)
	project_ty	*pp;
	string_ty	*user_name;
{
	change_ty	*cp;

	trace(("project_developer_add(pp = %8.8lX, user_name = \"%s\")\n{\n"/*}*/,
		(long)pp, user_name->str_text));
	cp = project_change_get(pp);
	change_branch_developer_add(cp, user_name);
	trace((/*{*/"}\n"));
}


void
project_developer_remove(pp, user_name)
	project_ty	*pp;
	string_ty	*user_name;
{
	change_ty	*cp;

	trace(("project_developer_remove(pp = %8.8lX, user_name = \"%s\")\n{\n"/*}*/,
		(long)pp, user_name->str_text));
	cp = project_change_get(pp);
	change_branch_developer_remove(cp, user_name);
	trace((/*{*/"}\n"));
}


string_ty *
project_developer_nth(pp, n)
	project_ty	*pp;
	long		n;
{
	change_ty	*cp;
	string_ty	*result;

	trace(("project_developer_nth(pp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)pp, n));
	cp = project_change_get(pp);
	result = change_branch_developer_nth(cp, n);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


int
project_reviewer_query(pp, user_name)
	project_ty	*pp;
	string_ty	*user_name;
{
	change_ty	*cp;
	int		result;

	trace(("project_reviewer_query(pp = %8.8lX, user_name = \"%s\")\n{\n"/*}*/,
		(long)pp, user_name->str_text));
	cp = project_change_get(pp);
	result = change_branch_reviewer_query(cp, user_name);
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


void
project_reviewer_add(pp, user_name)
	project_ty	*pp;
	string_ty	*user_name;
{
	change_ty	*cp;

	trace(("project_reviewer_add(pp = %8.8lX, user_name = \"%s\")\n{\n"/*}*/,
		(long)pp, user_name->str_text));
	cp = project_change_get(pp);
	change_branch_reviewer_add(cp, user_name);
	trace((/*{*/"}\n"));
}


void
project_reviewer_remove(pp, user_name)
	project_ty	*pp;
	string_ty	*user_name;
{
	change_ty	*cp;

	trace(("project_reviewer_remove(pp = %8.8lX, user_name = \"%s\")\n{\n"/*}*/,
		(long)pp, user_name->str_text));
	cp = project_change_get(pp);
	change_branch_reviewer_remove(cp, user_name);
	trace((/*{*/"}\n"));
}


string_ty *
project_reviewer_nth(pp, n)
	project_ty	*pp;
	long		n;
{
	change_ty	*cp;
	string_ty	*result;

	trace(("project_reviewer_nth(pp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)pp, n));
	cp = project_change_get(pp);
	result = change_branch_reviewer_nth(cp, n);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


int
project_integrator_query(pp, user_name)
	project_ty	*pp;
	string_ty	*user_name;
{
	change_ty	*cp;
	int		result;

	trace(("project_integrator_query(pp = %8.8lX, user_name = \"%s\")\n{\n"/*}*/,
		(long)pp, user_name->str_text));
	cp = project_change_get(pp);
	result = change_branch_integrator_query(cp, user_name);
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


void
project_integrator_add(pp, user_name)
	project_ty	*pp;
	string_ty	*user_name;
{
	change_ty	*cp;

	trace(("project_integrator_add(pp = %8.8lX, user_name = \"%s\")\n{\n"/*}*/,
		(long)pp, user_name->str_text));
	cp = project_change_get(pp);
	change_branch_integrator_add(cp, user_name);
	trace((/*{*/"}\n"));
}


void
project_integrator_remove(pp, user_name)
	project_ty	*pp;
	string_ty	*user_name;
{
	change_ty	*cp;

	trace(("project_integrator_remove(pp = %8.8lX, user_name = \"%s\")\n{\n"/*}*/,
		(long)pp, user_name->str_text));
	cp = project_change_get(pp);
	change_branch_integrator_remove(cp, user_name);
	trace((/*{*/"}\n"));
}


string_ty *
project_integrator_nth(pp, n)
	project_ty	*pp;
	long		n;
{
	change_ty	*cp;
	string_ty	*result;

	trace(("project_integrator_nth(pp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)pp, n));
	cp = project_change_get(pp);
	result = change_branch_integrator_nth(cp, n);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


void
project_change_add(pp, change_number, is_a_branch)
	project_ty	*pp;
	long		change_number;
	int		is_a_branch;
{
	change_ty	*cp;

	trace(("project_change_add(pp = %8.8lX, change_number = %ld)\n{\n"/*}*/,
		(long)pp, change_number));
	cp = project_change_get(pp);
	change_branch_change_add(cp, change_number, is_a_branch);
	trace((/*{*/"}\n"));
}


void
project_change_remove(pp, change_number)
	project_ty	*pp;
	long		change_number;
{
	change_ty	*cp;

	trace(("project_change_remove(pp = %8.8lX, change_number = %ld)\n{\n"/*}*/,
		(long)pp, change_number));
	cp = project_change_get(pp);
	change_branch_change_remove(cp, change_number);
	trace((/*{*/"}\n"));
}


int
project_change_nth(pp, n, cnp)
	project_ty	*pp;
	long		n;
	long		*cnp;
{
	change_ty	*cp;
	int		result;

	trace(("project_change_nth(pp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)pp, n));
	cp = project_change_get(pp);
	result = change_branch_change_nth(cp, n, cnp);
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


long
project_next_change_number(pp, skip)
	project_ty	*pp;
	int		skip;
{
	change_ty	*cp;
	long		change_number;

	trace(("project_next_change_number(pp = %8.8lX)\n{\n"/*}*/, (long)pp));
	cp = project_change_get(pp);
	change_number = change_branch_next_change_number(cp, skip);
	trace(("return %ld;\n", change_number));
	trace((/*{*/"}\n"));
	return change_number;
}


long
project_last_change_integrated(pp)
	project_ty	*pp;
{
	change_ty	*cp;
	long		n;

	/*
	 * This function is used to determine if the project file list
	 * has (potentially) changed.  Hash all of the ancestors
	 * together.
	 */
	n = 0;
	while (pp)
	{
		cp = project_change_get(pp);
		n = n * 29 + change_history_last_change_integrated(cp);
		pp = pp->parent;
	}
	return n;
}


void
project_description_set(pp, s)
	project_ty	*pp;
	string_ty	*s;
{
	change_ty	*cp;
	cstate		cstate_data;

	/*
	 * set the trunk change state file
	 */
	trace(("project_description_set(pp = %8.8lX, s = \"%s\")\n{\n"/*}*/,
		(long)pp, s->str_text));
	cp = project_change_get(pp);
	cstate_data = change_cstate_get(cp);
	if (cstate_data->brief_description)
		str_free(cstate_data->brief_description);
	cstate_data->brief_description = str_copy(s);
	trace((/*{*/"}\n"));
}


string_ty *
project_description_get(pp)
	project_ty	*pp;
{
	change_ty	*cp;
	cstate		cstate_data;

	cp = project_change_get(pp);
	cstate_data = change_cstate_get(cp);
	return cstate_data->brief_description;
}


string_ty *
project_version_previous_get(pp)
	project_ty	*pp;
{
	change_ty	*cp;
	cstate		cstate_data;

	cp = project_change_get(pp);
	cstate_data = change_cstate_get(cp);
	return cstate_data->version_previous;
}


void
project_version_previous_set(pp, s)
	project_ty	*pp;
	string_ty	*s;
{
	change_ty	*cp;
	cstate		cstate_data;

	cp = project_change_get(pp);
	cstate_data = change_cstate_get(cp);
	if (cstate_data->version_previous)
		str_free(cstate_data->version_previous);
	if (s)
		cstate_data->version_previous = str_copy(s);
	else
		cstate_data->version_previous = 0;
}


void
project_umask_set(pp, n)
	project_ty	*pp;
	int		n;
{
	change_branch_umask_set(project_change_get(pp), n);
}


int
project_umask_get(pp)
	project_ty	*pp;
{
	return change_branch_umask_get(project_change_get(pp));
}


void
project_developer_may_review_set(pp, n)
	project_ty	*pp;
	int		n;
{
	change_branch_developer_may_review_set(project_change_get(pp), n);
}


int
project_developer_may_review_get(pp)
	project_ty	*pp;
{
	return change_branch_developer_may_review_get(project_change_get(pp));
}


void
project_developer_may_integrate_set(pp, n)
	project_ty	*pp;
	int		n;
{
	change_branch_developer_may_integrate_set(project_change_get(pp), n);
}


int
project_developer_may_integrate_get(pp)
	project_ty	*pp;
{
	return change_branch_developer_may_integrate_get(project_change_get(pp));
}


void
project_reviewer_may_integrate_set(pp, n)
	project_ty	*pp;
	int		n;
{
	change_branch_reviewer_may_integrate_set(project_change_get(pp), n);
}

int
project_reviewer_may_integrate_get(pp)
	project_ty	*pp;
{
	return change_branch_reviewer_may_integrate_get(project_change_get(pp));
}


void
project_developers_may_create_changes_set(pp, n)
	project_ty	*pp;
	int		n;
{
	change_branch_developers_may_create_changes_set(project_change_get(pp), n);
}


int
project_developers_may_create_changes_get(pp)
	project_ty	*pp;
{
	return change_branch_developers_may_create_changes_get(project_change_get(pp));
}


void
project_forced_develop_begin_notify_command_set(pp, s)
	project_ty	*pp;
	string_ty	*s;
{
	change_branch_forced_develop_begin_notify_command_set(project_change_get(pp), s);
}


string_ty *
project_forced_develop_begin_notify_command_get(pp)
	project_ty	*pp;
{
	return change_branch_forced_develop_begin_notify_command_get(project_change_get(pp));
}


void
project_develop_end_notify_command_set(pp, s)
	project_ty	*pp;
	string_ty	*s;
{
	change_branch_develop_end_notify_command_set(project_change_get(pp), s);
}


string_ty *
project_develop_end_notify_command_get(pp)
	project_ty	*pp;
{
	return change_branch_develop_end_notify_command_get(project_change_get(pp));
}


void
project_develop_end_undo_notify_command_set(pp, s)
	project_ty	*pp;
	string_ty	*s;
{
	change_branch_develop_end_undo_notify_command_set(project_change_get(pp), s);
}


string_ty *
project_develop_end_undo_notify_command_get(pp)
	project_ty	*pp;
{
	return change_branch_develop_end_undo_notify_command_get(project_change_get(pp));
}


void
project_review_pass_notify_command_set(pp, s)
	project_ty	*pp;
	string_ty	*s;
{
	change_branch_review_pass_notify_command_set(project_change_get(pp), s);
}


string_ty *
project_review_pass_notify_command_get(pp)
	project_ty	*pp;
{
	return change_branch_review_pass_notify_command_get(project_change_get(pp));
}


void
project_review_pass_undo_notify_command_set(pp, s)
	project_ty	*pp;
	string_ty	*s;
{
	change_branch_review_pass_undo_notify_command_set(project_change_get(pp), s);
}


string_ty *
project_review_pass_undo_notify_command_get(pp)
	project_ty	*pp;
{
	return change_branch_review_pass_undo_notify_command_get(project_change_get(pp));
}


void
project_review_fail_notify_command_set(pp, s)
	project_ty	*pp;
	string_ty	*s;
{
	change_branch_review_fail_notify_command_set(project_change_get(pp), s);
}

string_ty *
project_review_fail_notify_command_get(pp)
	project_ty	*pp;
{
	return change_branch_review_fail_notify_command_get(project_change_get(pp));
}


void
project_integrate_pass_notify_command_set(pp, s)
	project_ty	*pp;
	string_ty	*s;
{
	change_branch_integrate_pass_notify_command_set(project_change_get(pp), s);
}

string_ty *
project_integrate_pass_notify_command_get(pp)
	project_ty	*pp;
{
	return change_branch_integrate_pass_notify_command_get(project_change_get(pp));
}


void
project_integrate_fail_notify_command_set(pp, s)
	project_ty	*pp;
	string_ty	*s;
{
	change_branch_integrate_fail_notify_command_set(project_change_get(pp), s);
}

string_ty *
project_integrate_fail_notify_command_get(pp)
	project_ty	*pp;
{
	return change_branch_integrate_fail_notify_command_get(project_change_get(pp));
}


void
project_default_development_directory_set(pp, s)
	project_ty	*pp;
	string_ty	*s;
{
	change_branch_default_development_directory_set(project_change_get(pp), s);
}

string_ty *
project_default_development_directory_get(pp)
	project_ty	*pp;
{
	return change_branch_default_development_directory_get(project_change_get(pp));
}


void
project_default_test_exemption_set(pp, n)
	project_ty	*pp;
	int		n;
{
	change_branch_default_test_exemption_set(project_change_get(pp), n);
}


int
project_default_test_exemption_get(pp)
	project_ty	*pp;
{
	return change_branch_default_test_exemption_get(project_change_get(pp));
}
