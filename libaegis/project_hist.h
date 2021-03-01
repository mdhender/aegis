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
 * MANIFEST: interface definition for aegis/project_hist.c
 */

#ifndef AEGIS_PROJECT_HIST_H
#define AEGIS_PROJECT_HIST_H

#include <project.h>

struct change_ty; /* existence */
struct string_list_ty; /* existence */

int project_history_delta_validate _((project_ty *, long));
time_t project_history_delta_to_timestamp _((project_ty *, time_t));
long project_history_delta_latest _((project_ty *));
long project_history_delta_by_name _((project_ty *, string_ty *, int));
void project_history_delta_name_delete _((project_ty *, string_ty *));
void project_history_delta_name_add _((project_ty *, long, string_ty *));
long project_current_integration_get _((project_ty *));
void project_current_integration_set _((project_ty *, long));
long project_next_delta_number _((project_ty *));
long project_change_number_to_delta_number _((project_ty *, long));
void project_copyright_years_merge _((project_ty *, struct change_ty *));
void project_copyright_years_get _((project_ty *pp, int *ary, int ary_len_max,
	int *ary_len));
void project_copyright_year_append _((project_ty *, int));
void project_history_new _((project_ty *pp, long dn, long cn));
int project_history_nth _((project_ty *pp, long n, long *cn, long *dn,
	struct string_list_ty *name));
long project_last_change_integrated _((project_ty *));

int project_administrator_query _((project_ty *, string_ty *));
void project_administrator_add _((project_ty *, string_ty *));
void project_administrator_remove _((project_ty *, string_ty *));
string_ty *project_administrator_nth _((project_ty *, long));
int project_developer_query _((project_ty *, string_ty *));
void project_developer_add _((project_ty *, string_ty *));
void project_developer_remove _((project_ty *, string_ty *));
string_ty *project_developer_nth _((project_ty *, long));
int project_reviewer_query _((project_ty *, string_ty *));
void project_reviewer_add _((project_ty *, string_ty *));
void project_reviewer_remove _((project_ty *, string_ty *));
string_ty *project_reviewer_nth _((project_ty *, long));
int project_integrator_query _((project_ty *, string_ty *));
void project_integrator_add _((project_ty *, string_ty *));
void project_integrator_remove _((project_ty *, string_ty *));
string_ty *project_integrator_nth _((project_ty *, long));

void project_change_add _((project_ty *, long, int));
void project_change_remove _((project_ty *, long));
int project_change_nth _((project_ty *, long, long *));
long project_next_change_number _((project_ty *pp, int skip));

void project_description_set _((project_ty *, string_ty *));
string_ty *project_description_get _((project_ty *));
void project_version_previous_set _((project_ty *, string_ty *));
string_ty *project_version_previous_get _((project_ty *));

void project_umask_set _((project_ty *, int));
int project_umask_get _((project_ty *));
void project_developer_may_review_set _((project_ty *, int));
int project_developer_may_review_get _((project_ty *));
void project_developer_may_integrate_set _((project_ty *, int));
int project_developer_may_integrate_get _((project_ty *));
void project_reviewer_may_integrate_set _((project_ty *, int));
int project_reviewer_may_integrate_get _((project_ty *));
void project_developers_may_create_changes_set _((project_ty *, int));
int project_developers_may_create_changes_get _((project_ty *));
void project_forced_develop_begin_notify_command_set _((project_ty *,
	string_ty *));
string_ty *project_forced_develop_begin_notify_command_get _((project_ty *));
void project_develop_end_notify_command_set _((project_ty *, string_ty *));
string_ty *project_develop_end_notify_command_get _((project_ty *));
void project_develop_end_undo_notify_command_set _((project_ty *, string_ty *));
string_ty *project_develop_end_undo_notify_command_get _((project_ty *));
void project_review_begin_notify_command_set _((project_ty *, string_ty *));
string_ty *project_review_begin_notify_command_get _((project_ty *));
void project_review_begin_undo_notify_command_set _((project_ty *,
	string_ty *));
string_ty *project_review_begin_undo_notify_command_get _((project_ty *));
void project_review_pass_notify_command_set _((project_ty *, string_ty *));
string_ty *project_review_pass_notify_command_get _((project_ty *));
void project_review_pass_undo_notify_command_set _((project_ty *, string_ty *));
string_ty *project_review_pass_undo_notify_command_get _((project_ty *));
void project_review_fail_notify_command_set _((project_ty *, string_ty *));
string_ty *project_review_fail_notify_command_get _((project_ty *));
void project_integrate_pass_notify_command_set _((project_ty *, string_ty *));
string_ty *project_integrate_pass_notify_command_get _((project_ty *));
void project_integrate_fail_notify_command_set _((project_ty *, string_ty *));
string_ty *project_integrate_fail_notify_command_get _((project_ty *));
void project_default_development_directory_set _((project_ty *, string_ty *));
string_ty *project_default_development_directory_get _((project_ty *));
void project_default_test_exemption_set _((project_ty *, int));
int project_default_test_exemption_get _((project_ty *));

#endif /* AEGIS_PROJECT_HIST_H */
