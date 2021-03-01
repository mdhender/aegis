//
//	aegis - project change supervisor
//	Copyright (C) 1995-1999, 2001, 2002, 2004-2008 Peter Miller
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

#ifndef AEGIS_CHANGE_BRANCH_H
#define AEGIS_CHANGE_BRANCH_H

#include <libaegis/change.h>

struct string_list_ty; // forward
struct change_list_ty; // forward

bool change_is_a_branch(change::pointer );
bool change_was_a_branch(change::pointer );

void change_branch_new(change::pointer );
bool change_history_delta_validate(change::pointer , long);
time_t change_history_delta_to_timestamp(struct project_ty *, long);
long change_history_timestamp_to_delta(struct project_ty *, time_t);
long change_history_delta_latest(change::pointer );
long change_history_delta_by_name(change::pointer , string_ty *, int);
long change_history_change_by_name(change::pointer , string_ty *, int);
long change_history_change_by_delta(change::pointer , long);
long change_history_change_by_timestamp(struct project_ty *, time_t);
void change_history_delta_name_delete(change::pointer , string_ty *);
void change_history_delta_name_add(change::pointer , long, string_ty *);
long change_history_last_change_integrated(change::pointer );
long change_current_integration_get(change::pointer );
void change_current_integration_set(change::pointer , long);
long change_branch_next_delta_number(change::pointer );
void change_copyright_years_now(change::pointer );
void change_copyright_year_append(change::pointer , int);
void change_copyright_years_merge(change::pointer , change::pointer );
void change_copyright_years_slurp(change::pointer cp, int *ary, int ary_len_max,
	int *ary_len);
void change_copyright_years_get(change::pointer cp, int *ary, int ary_len_max,
	int *ary_len);
void change_branch_history_new(change::pointer cp, long dn, long cn);
int change_branch_history_nth(change::pointer cp, long n, long *cnp, long *dnp,
	struct string_list_ty *name);

bool change_branch_administrator_query(change::pointer , string_ty *);
void change_branch_administrator_add(change::pointer , string_ty *);
void change_branch_administrator_remove(change::pointer , string_ty *);
string_ty *change_branch_administrator_nth(change::pointer , long);
bool change_branch_developer_query(change::pointer , string_ty *);
void change_branch_developer_add(change::pointer , string_ty *);
void change_branch_developer_remove(change::pointer , string_ty *);
string_ty *change_branch_developer_nth(change::pointer , long);
bool change_branch_reviewer_query(change::pointer , string_ty *);
void change_branch_reviewer_add(change::pointer , string_ty *);
void change_branch_reviewer_remove(change::pointer , string_ty *);
string_ty *change_branch_reviewer_nth(change::pointer , long);
bool change_branch_integrator_query(change::pointer , string_ty *);
void change_branch_integrator_add(change::pointer , string_ty *);
void change_branch_integrator_remove(change::pointer , string_ty *);
string_ty *change_branch_integrator_nth(change::pointer , long);

void change_branch_change_add(change::pointer , long, int);
void change_branch_change_remove(change::pointer , long);
void change_branch_sub_branch_list_get(change::pointer , long **, size_t *);
int change_branch_change_nth(change::pointer , long, long *);
long change_branch_next_change_number(change::pointer cp, int skip);
int change_branch_change_number_in_use(change::pointer , long);

void change_branch_umask_set(change::pointer , int);
int change_branch_umask_get(change::pointer );
void change_branch_developer_may_review_set(change::pointer , bool);
bool change_branch_developer_may_review_get(change::pointer );
void change_branch_developer_may_integrate_set(change::pointer , bool);
bool change_branch_developer_may_integrate_get(change::pointer );
void change_branch_reviewer_may_integrate_set(change::pointer , bool);
bool change_branch_reviewer_may_integrate_get(change::pointer );
void change_branch_developers_may_create_changes_set(change::pointer , bool);
bool change_branch_developers_may_create_changes_get(change::pointer );
void change_branch_forced_develop_begin_notify_command_set(change::pointer ,
	string_ty *);
string_ty *change_branch_forced_develop_begin_notify_command_get
   (change::pointer );
void change_branch_develop_end_notify_command_set(change::pointer cp,
    string_ty *command);
string_ty *change_branch_develop_end_notify_command_get(change::pointer );
void change_branch_develop_end_undo_notify_command_set(change::pointer ,
	string_ty *);
string_ty *change_branch_develop_end_undo_notify_command_get(change::pointer );
void change_branch_review_begin_notify_command_set(change::pointer ,
	string_ty *);
string_ty *change_branch_review_begin_notify_command_get(change::pointer );
void change_branch_review_begin_undo_notify_command_set(change::pointer ,
	string_ty *);
string_ty *change_branch_review_begin_undo_notify_command_get(change::pointer );
void change_branch_review_pass_notify_command_set(change::pointer cp,
    string_ty *command);
string_ty *change_branch_review_pass_notify_command_get(change::pointer );
void change_branch_review_pass_undo_notify_command_set(change::pointer ,
	string_ty *);
string_ty *change_branch_review_pass_undo_notify_command_get(change::pointer );
void change_branch_review_fail_notify_command_set(change::pointer cp,
    string_ty *command);
string_ty *change_branch_review_fail_notify_command_get(change::pointer );
void change_branch_integrate_pass_notify_command_set(change::pointer ,
	string_ty *);
string_ty *change_branch_integrate_pass_notify_command_get(change::pointer );
void change_branch_integrate_fail_notify_command_set(change::pointer ,
	string_ty *);
string_ty *change_branch_integrate_fail_notify_command_get(change::pointer );
void change_branch_default_development_directory_set(change::pointer ,
	string_ty *);
string_ty *change_branch_default_development_directory_get(change::pointer );

/**
  * The change_branch_default_test_exemption_set function is used to set
  * the default_test_exemption attribute of a branch.
  *
  * @param cp
  *     The branch being modified.
  * @param yesno
  *     The state to set the attribute.
  */
void change_branch_default_test_exemption_set(change::pointer cp, bool yesno);

/**
  * The change_branch_default_test_exemption_get function is used to
  * obtain the current state of a branch's default_test_exemption
  * attribute.
  *
  * @param cp
  *     The branch being quesried.
  * @returns
  *     The state of the attribute.
  */
bool change_branch_default_test_exemption_get(change::pointer cp);

/**
  * The change_branch_default_test_regression_exemption_set function is
  * used to set the default_test_regression_exemption attribute of a
  * branch.
  *
  * @param cp
  *     The branch being modified.
  * @param yesno
  *     The state to set the attribute.
  */
void change_branch_default_test_regression_exemption_set(change::pointer cp,
    bool yesno);

/**
  * The change_branch_default_test_regression_exemption_get
  * function is used to obtain the current state of a branch's
  * default_test_regression_exemption attribute.
  *
  * @param cp
  *     The branch being quesried.
  * @returns
  *     The state of the attribute.
  */
bool change_branch_default_test_regression_exemption_get(change::pointer cp);

long change_branch_minimum_change_number_get(change::pointer );
void change_branch_minimum_change_number_set(change::pointer , long);
bool change_branch_reuse_change_numbers_get(change::pointer );
long change_branch_minimum_branch_number_get(change::pointer );
void change_branch_minimum_branch_number_set(change::pointer , long);
void change_branch_reuse_change_numbers_set(change::pointer , bool);
bool change_branch_skip_unlucky_get(change::pointer );
void change_branch_skip_unlucky_set(change::pointer , bool);
bool change_branch_compress_database_get(change::pointer );
void change_branch_compress_database_set(change::pointer , bool);
string_ty *change_version_get(change::pointer );
int change_branch_develop_end_action_get(change::pointer );
void change_branch_develop_end_action_set(change::pointer , int);
bool change_branch_protect_development_directory_get(change::pointer );
void change_branch_protect_development_directory_set(change::pointer , bool);
time_t change_completion_timestamp(change::pointer );

/**
  * The change_branch_uuid_find function is used to locate a change by
  * its UUID.  This function will recurse down the branch tree.  It is
  * possible to give a leading prefix - this is shorter, and easier for
  * humans to type.  We return all such matches, although usually it's
  * an error if there is more than one.
  *
  * @param pp
  *     The change or branch to search.
  * @param uuid
  *     The change UUID to search for.
  * @param result
  *     This is the list of changes which match this (partial) UUID specified.
  */
void change_branch_uuid_find(change::pointer pp, string_ty *uuid,
    change_list_ty &result);

#endif // AEGIS_CHANGE_BRANCH_H
