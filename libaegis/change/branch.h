/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995-1999, 2001, 2002, 2004 Peter Miller;
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
 * MANIFEST: interface definition for aegis/change_bran.c
 */

#ifndef AEGIS_CHANGE_BRAN_H
#define AEGIS_CHANGE_BRAN_H

#include <change.h>

struct string_list_ty; /* existence */

bool change_is_a_branch(change_ty *);
bool change_was_a_branch(change_ty *);

void change_branch_new(change_ty *);
bool change_history_delta_validate(change_ty *, long);
time_t change_history_delta_to_timestamp(struct project_ty *, long);
long change_history_timestamp_to_delta(struct project_ty *, time_t);
long change_history_delta_latest(change_ty *);
long change_history_delta_by_name(change_ty *, string_ty *, int);
long change_history_change_by_name(change_ty *, string_ty *, int);
long change_history_change_by_delta(change_ty *, long);
long change_history_change_by_timestamp(struct project_ty *, time_t);
void change_history_delta_name_delete(change_ty *, string_ty *);
void change_history_delta_name_add(change_ty *, long, string_ty *);
long change_history_last_change_integrated(change_ty *);
long change_current_integration_get(change_ty *);
void change_current_integration_set(change_ty *, long);
long change_branch_next_delta_number(change_ty *);
void change_copyright_years_now(change_ty *);
void change_copyright_year_append(change_ty *, int);
void change_copyright_years_merge(change_ty *, change_ty *);
void change_copyright_years_get(change_ty *cp, int *ary, int ary_len_max,
	int *ary_len);
void change_branch_history_new(change_ty *cp, long dn, long cn);
int change_branch_history_nth(change_ty *cp, long n, long *cnp, long *dnp,
	struct string_list_ty *name);

bool change_branch_administrator_query(change_ty *, string_ty *);
void change_branch_administrator_add(change_ty *, string_ty *);
void change_branch_administrator_remove(change_ty *, string_ty *);
string_ty *change_branch_administrator_nth(change_ty *, long);
bool change_branch_developer_query(change_ty *, string_ty *);
void change_branch_developer_add(change_ty *, string_ty *);
void change_branch_developer_remove(change_ty *, string_ty *);
string_ty *change_branch_developer_nth(change_ty *, long);
bool change_branch_reviewer_query(change_ty *, string_ty *);
void change_branch_reviewer_add(change_ty *, string_ty *);
void change_branch_reviewer_remove(change_ty *, string_ty *);
string_ty *change_branch_reviewer_nth(change_ty *, long);
bool change_branch_integrator_query(change_ty *, string_ty *);
void change_branch_integrator_add(change_ty *, string_ty *);
void change_branch_integrator_remove(change_ty *, string_ty *);
string_ty *change_branch_integrator_nth(change_ty *, long);

void change_branch_change_add(change_ty *, long, int);
void change_branch_change_remove(change_ty *, long);
void change_branch_sub_branch_list_get(change_ty *, long **, size_t *);
int change_branch_change_nth(change_ty *, long, long *);
long change_branch_next_change_number(change_ty *cp, int skip);
int change_branch_change_number_in_use(change_ty *, long);

void change_branch_umask_set(change_ty *, int);
int change_branch_umask_get(change_ty *);
void change_branch_developer_may_review_set(change_ty *, bool);
bool change_branch_developer_may_review_get(change_ty *);
void change_branch_developer_may_integrate_set(change_ty *, bool);
bool change_branch_developer_may_integrate_get(change_ty *);
void change_branch_reviewer_may_integrate_set(change_ty *, bool);
bool change_branch_reviewer_may_integrate_get(change_ty *);
void change_branch_developers_may_create_changes_set(change_ty *, bool);
bool change_branch_developers_may_create_changes_get(change_ty *);
void change_branch_forced_develop_begin_notify_command_set(change_ty *,
	string_ty *);
string_ty *change_branch_forced_develop_begin_notify_command_get
   (change_ty *);
void change_branch_develop_end_notify_command_set(change_ty *, string_ty *);
string_ty *change_branch_develop_end_notify_command_get(change_ty *);
void change_branch_develop_end_undo_notify_command_set(change_ty *,
	string_ty *);
string_ty *change_branch_develop_end_undo_notify_command_get(change_ty *);
void change_branch_review_begin_notify_command_set(change_ty *,
	string_ty *);
string_ty *change_branch_review_begin_notify_command_get(change_ty *);
void change_branch_review_begin_undo_notify_command_set(change_ty *,
	string_ty *);
string_ty *change_branch_review_begin_undo_notify_command_get(change_ty *);
void change_branch_review_pass_notify_command_set(change_ty *, string_ty *);
string_ty *change_branch_review_pass_notify_command_get(change_ty *);
void change_branch_review_pass_undo_notify_command_set(change_ty *,
	string_ty *);
string_ty *change_branch_review_pass_undo_notify_command_get(change_ty *);
void change_branch_review_fail_notify_command_set(change_ty *, string_ty *);
string_ty *change_branch_review_fail_notify_command_get(change_ty *);
void change_branch_integrate_pass_notify_command_set(change_ty *,
	string_ty *);
string_ty *change_branch_integrate_pass_notify_command_get(change_ty *);
void change_branch_integrate_fail_notify_command_set(change_ty *,
	string_ty *);
string_ty *change_branch_integrate_fail_notify_command_get(change_ty *);
void change_branch_default_development_directory_set(change_ty *,
	string_ty *);
string_ty *change_branch_default_development_directory_get(change_ty *);
void change_branch_default_test_exemption_set(change_ty *, bool);
bool change_branch_default_test_exemption_get(change_ty *);
long change_branch_minimum_change_number_get(change_ty *);
void change_branch_minimum_change_number_set(change_ty *, long);
bool change_branch_reuse_change_numbers_get(change_ty *);
long change_branch_minimum_branch_number_get(change_ty *);
void change_branch_minimum_branch_number_set(change_ty *, long);
void change_branch_reuse_change_numbers_set(change_ty *, bool);
bool change_branch_skip_unlucky_get(change_ty *);
void change_branch_skip_unlucky_set(change_ty *, bool);
bool change_branch_compress_database_get(change_ty *);
void change_branch_compress_database_set(change_ty *, bool);
string_ty *change_version_get(change_ty *);
int change_branch_develop_end_action_get(change_ty *);
void change_branch_develop_end_action_set(change_ty *, int);
bool change_branch_protect_development_directory_get(change_ty *);
void change_branch_protect_development_directory_set(change_ty *, bool);
time_t change_completion_timestamp(change_ty *);

/**
  * The change_branch_uuid_find function is used to locate a change by
  * its UUID.  This function will recurse down the branch tree.
  *
  * @param pp
  *     The change or branch to search.
  * @param uuid
  *     The change UUID to search for.
  * @returns
  *     a pointer to the change with th given UUID, or NULL if no change
  *     has the given UUID.
  */
change_ty *change_branch_uuid_find(change_ty *pp, string_ty *uuid);

#endif /* AEGIS_CHANGE_BRAN_H */
