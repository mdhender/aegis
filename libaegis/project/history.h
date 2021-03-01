//
//      aegis - project change supervisor
//      Copyright (C) 1995-1998, 2001-2003, 2005-2008, 2011, 2012 Peter Miller
//      Copyright (C) 2008 Walter Franzini
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

#ifndef AEGIS_PROJECT_HISTORY_H
#define AEGIS_PROJECT_HISTORY_H

#include <libaegis/project.h>

struct string_list_ty; // existence

bool project_history_delta_validate(project *, long);
time_t project_history_delta_to_timestamp(project *, long);
long project_history_timestamp_to_delta(project *, time_t);
long project_history_delta_latest(project *);
long project_history_delta_by_name(project *, string_ty *, int);
long project_history_change_by_name(project *, string_ty *, int);
long project_history_change_by_timestamp(project *, time_t);
long project_history_change_by_delta(project *, long);
void project_history_delta_name_delete(project *, string_ty *);
void project_history_delta_name_add(project *, long, string_ty *);
long project_current_integration_get(project *);
void project_current_integration_set(project *, long);
long project_next_delta_number(project *);
long project_change_number_to_delta_number(project *, long);
long project_delta_number_to_change_number(project *, long);
void project_copyright_years_merge(project *, change::pointer );
void project_copyright_years_get(project *pp, int *ary, int ary_len_max,
        int *ary_len);
void project_copyright_year_append(project *, int);
void project_history_new(project *pp, long dn, long cn, string_ty *, time_t,
    bool);
int project_history_nth(project *pp, long n, long *cn, long *dn,
        struct string_list_ty *name);
long project_last_change_integrated(project *);

bool project_administrator_query(project *pp, string_ty *name) DEPRECATED;
bool project_administrator_query(project *pp, const nstring &name);

void project_administrator_add(project *pp, string_ty *name) DEPRECATED;
void project_administrator_add(project *pp, const nstring &name);

void project_administrator_remove(project *pp, string_ty *name) DEPRECATED;
void project_administrator_remove(project *pp, const nstring &name);

string_ty *project_administrator_nth(project *, long);

bool project_developer_query(project *pp, string_ty *name) DEPRECATED;
bool project_developer_query(project *pp, const nstring &name);

void project_developer_add(project *pp, string_ty *name) DEPRECATED;
void project_developer_add(project *pp, const nstring &name);

void project_developer_remove(project *pp, string_ty *name) DEPRECATED;
void project_developer_remove(project *pp, const nstring &name);

string_ty *project_developer_nth(project *, long);

bool project_reviewer_query(project *pp, string_ty *name) DEPRECATED;
bool project_reviewer_query(project *pp, const nstring &name);

void project_reviewer_add(project *pp, string_ty *name) DEPRECATED;
void project_reviewer_add(project *pp, const nstring &name);

void project_reviewer_remove(project *pp, string_ty *name) DEPRECATED;
void project_reviewer_remove(project *pp, const nstring &name);

string_ty *project_reviewer_nth(project *, long);

bool project_integrator_query(project *, string_ty *) DEPRECATED;
bool project_integrator_query(project *pp, const nstring &name);

void project_integrator_add(project *pp, string_ty *name) DEPRECATED;
void project_integrator_add(project *pp, const nstring &name);

void project_integrator_remove(project *pp, string_ty *name) DEPRECATED;
void project_integrator_remove(project *pp, const nstring &name);

string_ty *project_integrator_nth(project *, long);

void project_change_add(project *, long, int);
void project_change_remove(project *, long);
int project_change_nth(project *, long, long *);
long project_next_change_number(project *pp, int skip);

void project_description_set(project *, string_ty *);
nstring project_description_get(project *);
void project_version_previous_set(project *, string_ty *);
string_ty *project_version_previous_get(project *);

void project_umask_set(project *, int);
int project_umask_get(project *);
void project_developer_may_review_set(project *, bool);
bool project_developer_may_review_get(project *);
void project_developer_may_integrate_set(project *, bool);
bool project_developer_may_integrate_get(project *);
void project_reviewer_may_integrate_set(project *, bool);
bool project_reviewer_may_integrate_get(project *);
void project_developers_may_create_changes_set(project *, bool);
bool project_developers_may_create_changes_get(project *);
void project_forced_develop_begin_notify_command_set(project *,
        string_ty *);
string_ty *project_forced_develop_begin_notify_command_get(project *);
void project_develop_end_notify_command_set(project *, string_ty *);
string_ty *project_develop_end_notify_command_get(project *);
void project_develop_end_undo_notify_command_set(project *, string_ty *);
string_ty *project_develop_end_undo_notify_command_get(project *);
void project_review_begin_notify_command_set(project *, string_ty *);
string_ty *project_review_begin_notify_command_get(project *);
void project_review_begin_undo_notify_command_set(project *,
        string_ty *);
string_ty *project_review_begin_undo_notify_command_get(project *);
void project_review_pass_notify_command_set(project *, string_ty *);
string_ty *project_review_pass_notify_command_get(project *);
void project_review_pass_undo_notify_command_set(project *, string_ty *);
string_ty *project_review_pass_undo_notify_command_get(project *);
void project_review_fail_notify_command_set(project *, string_ty *);
string_ty *project_review_fail_notify_command_get(project *);
void project_integrate_pass_notify_command_set(project *, string_ty *);
string_ty *project_integrate_pass_notify_command_get(project *);
void project_integrate_fail_notify_command_set(project *, string_ty *);
string_ty *project_integrate_fail_notify_command_get(project *);
void project_default_development_directory_set(project *, string_ty *);
string_ty *project_default_development_directory_get(project *);

/**
  * The project_default_test_exemption_set function is used to set the
  * test exemption attribute of a project.
  *
  * @param pp
  *     The project being operated on
  * @param yesno
  *     The state to set the attribute
  */
void project_default_test_exemption_set(project *pp, bool yesno);

/**
  * The project_default_test_exemption_get function is used to obtain
  * the current setting of the test exemption attribute of a project.
  *
  * @param pp
  *     The project being queried
  * @returns
  *     The current state of the attribute
  */
bool project_default_test_exemption_get(project *pp);

/**
  * The project_default_test_regression_exemption_set function is used
  * to set the regression test exemption attribute of a project.
  *
  * @param pp
  *     The project being operated on
  * @param yesno
  *     The state to set the attribute
  */
void project_default_test_regression_exemption_set(project *pp, bool yesno);

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
bool project_default_test_regression_exemption_get(project *pp);

#endif // AEGIS_PROJECT_HISTORY_H
// vim: set ts=8 sw=4 et :
