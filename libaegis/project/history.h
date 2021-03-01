//
//	aegis - project change supervisor
//	Copyright (C) 1995-1998, 2001-2003, 2005-2008 Peter Miller
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

#ifndef AEGIS_PROJECT_HISTORY_H
#define AEGIS_PROJECT_HISTORY_H

#include <libaegis/project.h>

struct string_list_ty; // existence

bool project_history_delta_validate(project_ty *, long);
time_t project_history_delta_to_timestamp(project_ty *, long);
long project_history_timestamp_to_delta(project_ty *, time_t);
long project_history_delta_latest(project_ty *);
long project_history_delta_by_name(project_ty *, string_ty *, int);
long project_history_change_by_name(project_ty *, string_ty *, int);
long project_history_change_by_timestamp(project_ty *, time_t);
long project_history_change_by_delta(project_ty *, long);
void project_history_delta_name_delete(project_ty *, string_ty *);
void project_history_delta_name_add(project_ty *, long, string_ty *);
long project_current_integration_get(project_ty *);
void project_current_integration_set(project_ty *, long);
long project_next_delta_number(project_ty *);
long project_change_number_to_delta_number(project_ty *, long);
long project_delta_number_to_change_number(project_ty *, long);
void project_copyright_years_merge(project_ty *, change::pointer );
void project_copyright_years_get(project_ty *pp, int *ary, int ary_len_max,
	int *ary_len);
void project_copyright_year_append(project_ty *, int);
void project_history_new(project_ty *pp, long dn, long cn);
int project_history_nth(project_ty *pp, long n, long *cn, long *dn,
	struct string_list_ty *name);
long project_last_change_integrated(project_ty *);

bool project_administrator_query(project_ty *pp, string_ty *name) DEPRECATED;
bool project_administrator_query(project_ty *pp, const nstring &name);

void project_administrator_add(project_ty *pp, string_ty *name) DEPRECATED;
void project_administrator_add(project_ty *pp, const nstring &name);

void project_administrator_remove(project_ty *pp, string_ty *name) DEPRECATED;
void project_administrator_remove(project_ty *pp, const nstring &name);

string_ty *project_administrator_nth(project_ty *, long);

bool project_developer_query(project_ty *pp, string_ty *name) DEPRECATED;
bool project_developer_query(project_ty *pp, const nstring &name);

void project_developer_add(project_ty *pp, string_ty *name) DEPRECATED;
void project_developer_add(project_ty *pp, const nstring &name);

void project_developer_remove(project_ty *pp, string_ty *name) DEPRECATED;
void project_developer_remove(project_ty *pp, const nstring &name);

string_ty *project_developer_nth(project_ty *, long);

bool project_reviewer_query(project_ty *pp, string_ty *name) DEPRECATED;
bool project_reviewer_query(project_ty *pp, const nstring &name);

void project_reviewer_add(project_ty *pp, string_ty *name) DEPRECATED;
void project_reviewer_add(project_ty *pp, const nstring &name);

void project_reviewer_remove(project_ty *pp, string_ty *name) DEPRECATED;
void project_reviewer_remove(project_ty *pp, const nstring &name);

string_ty *project_reviewer_nth(project_ty *, long);

bool project_integrator_query(project_ty *, string_ty *) DEPRECATED;
bool project_integrator_query(project_ty *pp, const nstring &name);

void project_integrator_add(project_ty *pp, string_ty *name) DEPRECATED;
void project_integrator_add(project_ty *pp, const nstring &name);

void project_integrator_remove(project_ty *pp, string_ty *name) DEPRECATED;
void project_integrator_remove(project_ty *pp, const nstring &name);

string_ty *project_integrator_nth(project_ty *, long);

void project_change_add(project_ty *, long, int);
void project_change_remove(project_ty *, long);
int project_change_nth(project_ty *, long, long *);
long project_next_change_number(project_ty *pp, int skip);

void project_description_set(project_ty *, string_ty *);
string_ty *project_description_get(project_ty *);
void project_version_previous_set(project_ty *, string_ty *);
string_ty *project_version_previous_get(project_ty *);

void project_umask_set(project_ty *, int);
int project_umask_get(project_ty *);
void project_developer_may_review_set(project_ty *, bool);
bool project_developer_may_review_get(project_ty *);
void project_developer_may_integrate_set(project_ty *, bool);
bool project_developer_may_integrate_get(project_ty *);
void project_reviewer_may_integrate_set(project_ty *, bool);
bool project_reviewer_may_integrate_get(project_ty *);
void project_developers_may_create_changes_set(project_ty *, bool);
bool project_developers_may_create_changes_get(project_ty *);
void project_forced_develop_begin_notify_command_set(project_ty *,
	string_ty *);
string_ty *project_forced_develop_begin_notify_command_get(project_ty *);
void project_develop_end_notify_command_set(project_ty *, string_ty *);
string_ty *project_develop_end_notify_command_get(project_ty *);
void project_develop_end_undo_notify_command_set(project_ty *, string_ty *);
string_ty *project_develop_end_undo_notify_command_get(project_ty *);
void project_review_begin_notify_command_set(project_ty *, string_ty *);
string_ty *project_review_begin_notify_command_get(project_ty *);
void project_review_begin_undo_notify_command_set(project_ty *,
	string_ty *);
string_ty *project_review_begin_undo_notify_command_get(project_ty *);
void project_review_pass_notify_command_set(project_ty *, string_ty *);
string_ty *project_review_pass_notify_command_get(project_ty *);
void project_review_pass_undo_notify_command_set(project_ty *, string_ty *);
string_ty *project_review_pass_undo_notify_command_get(project_ty *);
void project_review_fail_notify_command_set(project_ty *, string_ty *);
string_ty *project_review_fail_notify_command_get(project_ty *);
void project_integrate_pass_notify_command_set(project_ty *, string_ty *);
string_ty *project_integrate_pass_notify_command_get(project_ty *);
void project_integrate_fail_notify_command_set(project_ty *, string_ty *);
string_ty *project_integrate_fail_notify_command_get(project_ty *);
void project_default_development_directory_set(project_ty *, string_ty *);
string_ty *project_default_development_directory_get(project_ty *);

/**
  * The project_default_test_exemption_set function is used to set the
  * test exemption attribute of a project.
  *
  * @param pp
  *     The project being operated on
  * @param yesno
  *     The state to set the attribute
  */
void project_default_test_exemption_set(project_ty *pp, bool yesno);

/**
  * The project_default_test_exemption_get function is used to obtain
  * the current setting of the test exemption attribute of a project.
  *
  * @param pp
  *     The project being queried
  * @returns
  *     The current state of the attribute
  */
bool project_default_test_exemption_get(project_ty *pp);

/**
  * The project_default_test_regression_exemption_set function is used
  * to set the regression test exemption attribute of a project.
  *
  * @param pp
  *     The project being operated on
  * @param yesno
  *     The state to set the attribute
  */
void project_default_test_regression_exemption_set(project_ty *pp, bool yesno);

/**
  * The project_default_test_regression_exemption_get function is used
  * to obtain the current setting of the regression test exemption
  * attribute of a project.
  *
  * @param pp
  *     The project being queried
  * @returns
  *     The current state of the attribute
  */
bool project_default_test_regression_exemption_get(project_ty *pp);

#endif // AEGIS_PROJECT_HISTORY_H
