/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995, 1996, 1997, 1998 Peter Miller;
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

int change_is_a_branch _((change_ty *));
int change_was_a_branch _((change_ty *));

void change_branch_new _((change_ty *));
int change_history_delta_validate _((change_ty *, long));
time_t change_history_delta_to_timestamp _((struct project_ty *, long));
long change_history_delta_latest _((change_ty *));
long change_history_delta_by_name _((change_ty *, string_ty *, int));
void change_history_delta_name_delete _((change_ty *, string_ty *));
void change_history_delta_name_add _((change_ty *, long, string_ty *));
long change_history_last_change_integrated _((change_ty *));
long change_current_integration_get _((change_ty *));
void change_current_integration_set _((change_ty *, long));
long change_delta_number_get _((change_ty *));
void change_copyright_years_now _((change_ty *));
void change_copyright_year_append _((change_ty *, int));
void change_copyright_years_merge _((change_ty *, change_ty *));
void change_copyright_years_get _((change_ty *cp, int *ary, int ary_len_max,
	int *ary_len));
void change_branch_history_new _((change_ty *cp, long dn, long cn));
int change_branch_history_nth _((change_ty *cp, long n, long *cnp, long *dnp,
	struct string_list_ty *name));

int change_branch_administrator_query _((change_ty *, string_ty *));
void change_branch_administrator_add _((change_ty *, string_ty *));
void change_branch_administrator_remove _((change_ty *, string_ty *));
string_ty *change_branch_administrator_nth _((change_ty *, long));
int change_branch_developer_query _((change_ty *, string_ty *));
void change_branch_developer_add _((change_ty *, string_ty *));
void change_branch_developer_remove _((change_ty *, string_ty *));
string_ty *change_branch_developer_nth _((change_ty *, long));
int change_branch_reviewer_query _((change_ty *, string_ty *));
void change_branch_reviewer_add _((change_ty *, string_ty *));
void change_branch_reviewer_remove _((change_ty *, string_ty *));
string_ty *change_branch_reviewer_nth _((change_ty *, long));
int change_branch_integrator_query _((change_ty *, string_ty *));
void change_branch_integrator_add _((change_ty *, string_ty *));
void change_branch_integrator_remove _((change_ty *, string_ty *));
string_ty *change_branch_integrator_nth _((change_ty *, long));

void change_branch_change_add _((change_ty *, long, int));
void change_branch_change_remove _((change_ty *, long));
void change_branch_sub_branch_list_get _((change_ty *, long **, size_t *));
int change_branch_change_nth _((change_ty *, long, long *));
long change_branch_next_change_number _((change_ty *cp, int skip));
int change_branch_change_number_in_use _((change_ty *, long));

string_ty *change_branch_delta_to_edit _((struct project_ty *, long delta,
	string_ty *fn));
string_ty *change_branch_delta_date_to_edit _((struct project_ty *, time_t
	delta_date, string_ty *fn));

void change_branch_umask_set _((change_ty *, int));
int change_branch_umask_get _((change_ty *));
void change_branch_developer_may_review_set _((change_ty *, int));
int change_branch_developer_may_review_get _((change_ty *));
void change_branch_developer_may_integrate_set _((change_ty *, int));
int change_branch_developer_may_integrate_get _((change_ty *));
void change_branch_reviewer_may_integrate_set _((change_ty *, int));
int change_branch_reviewer_may_integrate_get _((change_ty *));
void change_branch_developers_may_create_changes_set _((change_ty *, int));
int change_branch_developers_may_create_changes_get _((change_ty *));
void change_branch_forced_develop_begin_notify_command_set _((change_ty *,
	string_ty *));
string_ty *change_branch_forced_develop_begin_notify_command_get _((change_ty *));
void change_branch_develop_end_notify_command_set _((change_ty *, string_ty *));
string_ty *change_branch_develop_end_notify_command_get _((change_ty *));
void change_branch_develop_end_undo_notify_command_set _((change_ty *,
	string_ty *));
string_ty *change_branch_develop_end_undo_notify_command_get _((change_ty *));
void change_branch_review_pass_notify_command_set _((change_ty *, string_ty *));
string_ty *change_branch_review_pass_notify_command_get _((change_ty *));
void change_branch_review_pass_undo_notify_command_set _((change_ty *,
	string_ty *));
string_ty *change_branch_review_pass_undo_notify_command_get _((change_ty *));
void change_branch_review_fail_notify_command_set _((change_ty *, string_ty *));
string_ty *change_branch_review_fail_notify_command_get _((change_ty *));
void change_branch_integrate_pass_notify_command_set _((change_ty *,
	string_ty *));
string_ty *change_branch_integrate_pass_notify_command_get _((change_ty *));
void change_branch_integrate_fail_notify_command_set _((change_ty *,
	string_ty *));
string_ty *change_branch_integrate_fail_notify_command_get _((change_ty *));
void change_branch_default_development_directory_set _((change_ty *,
	string_ty *));
string_ty *change_branch_default_development_directory_get _((change_ty *));
void change_branch_default_test_exemption_set _((change_ty *, int));
int change_branch_default_test_exemption_get _((change_ty *));
long change_branch_minimum_change_number_get _((change_ty *));
void change_branch_minimum_change_number_set _((change_ty *, long));
string_ty *change_version_get _((change_ty *));

#endif /* AEGIS_CHANGE_BRAN_H */
