/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/change.c
 */

#ifndef LIBAEGIS_CHANGE_H
#define LIBAEGIS_CHANGE_H

#include <main.h>
#include <cstate.h>
#include <fstate.h>
#include <pconf.h>
#include <zero.h>

/*
 * Name of the project configuration file,
 * relative to the baseline/devdir root.
 */
#define THE_CONFIG_FILE "config"

/*
 * Define a magic number to represent the trunk transaction
 * of the project.  It has an exceptional file name.
 */
#define TRUNK_CHANGE_NUMBER ((long)((~(unsigned long)0) >> 1))

struct string_list_ty; /* existence */
struct user_ty; /* existence */
struct sub_context_ty; /* existence */

typedef struct change_ty change_ty;
struct change_ty
{
	long		reference_count;
	struct project_ty *pp;
	long		number;
	cstate		cstate_data;
	string_ty	*cstate_filename;
	int		cstate_is_a_new_file;
	fstate		fstate_data;
	struct symtab_ty *fstate_stp;
	string_ty	*fstate_filename;
	int		fstate_is_a_new_file;
	string_ty	*development_directory_unresolved;
	string_ty	*development_directory_resolved;
	string_ty	*integration_directory_unresolved;
	string_ty	*integration_directory_resolved;
	string_ty	*logfile;
	string_ty	*pconf_path;
	pconf		pconf_data;
	long		lock_magic;
	int		bogus;
	string_ty	*architecture_name;

	/*
	 * if you add to this structure, don't forget to update
	 * change_alloc() and change_free() in aegis/change.c
	 */
};

change_ty *change_alloc _((struct project_ty *, long));
void change_free _((change_ty *));
change_ty *change_copy _((change_ty *));
void change_bind_existing _((change_ty *));
void change_bind_new _((change_ty *));
cstate change_cstate_get _((change_ty *));
void change_cstate_write _((change_ty *));
cstate_history change_history_new _((change_ty *, struct user_ty *));
string_ty *change_developer_name _((change_ty *));
string_ty *change_reviewer_name _((change_ty *));
string_ty *change_integrator_name _((change_ty *));
void change_development_directory_set _((change_ty *, string_ty *));
void change_integration_directory_set _((change_ty *, string_ty *));
string_ty *change_development_directory_get _((change_ty *, int));
string_ty *change_integration_directory_get _((change_ty *, int));
string_ty *change_logfile_basename _((void));
string_ty *change_logfile_get _((change_ty *));
void change_cstate_lock_prepare _((change_ty *));
void change_error _((change_ty *, struct sub_context_ty *, char *));
void change_fatal _((change_ty *, struct sub_context_ty *, char *));
void change_verbose _((change_ty *, struct sub_context_ty *, char *));
string_ty *change_pconf_path_get _((change_ty *));
pconf change_pconf_get _((change_ty *, int));
void change_run_change_file_command _((change_ty *, struct string_list_ty *,
	struct user_ty *));
int change_run_project_file_command_needed _((change_ty *));
void change_run_project_file_command _((change_ty *, struct user_ty *));
void change_run_forced_develop_begin_notify_command _((change_ty *,
	struct user_ty *));
void change_run_develop_end_notify_command _((change_ty *));
void change_run_develop_end_undo_notify_command _((change_ty *));
void change_run_review_pass_notify_command _((change_ty *));
void change_run_review_pass_undo_notify_command _((change_ty *));
void change_run_review_fail_notify_command _((change_ty *));
void change_run_integrate_pass_notify_command _((change_ty *));
void change_run_integrate_fail_notify_command _((change_ty *));
void change_run_history_get_command _((change_ty *cp, string_ty *file_name,
	string_ty *edit_number, string_ty *output_file, struct user_ty *up));
void change_run_history_create_command _((change_ty *cp, string_ty *file_name));
void change_run_history_put_command _((change_ty *cp, string_ty *file_name));
string_ty *change_run_history_query_command _((change_ty *cp,
	string_ty *file_name));
void change_run_diff_command _((change_ty *cp, struct user_ty *up,
	string_ty *original, string_ty *input, string_ty *output));
void change_run_diff3_command _((change_ty *cp, struct user_ty *up,
	string_ty *original, string_ty *most_recent, string_ty *input,
	string_ty *output));
void change_run_merge_command _((change_ty *cp, struct user_ty *up,
	string_ty *original, string_ty *most_recent, string_ty *input,
	string_ty *output));
int change_has_merge_command _((change_ty *));
void change_run_integrate_begin_command _((change_ty *));
void change_run_develop_begin_command _((change_ty *, struct user_ty *));
int change_run_test_command _((change_ty *, struct user_ty *, string_ty *,
	string_ty *, int));
int change_run_development_test_command _((change_ty *, struct user_ty *,
	string_ty *, string_ty *, int));
void change_run_build_command _((change_ty *));
void change_run_development_build_command _((change_ty *, struct user_ty *,
	struct string_list_ty *));
string_ty *change_file_template _((change_ty *, string_ty *));
void change_become _((change_ty *));
void change_become_undo _((void));
int change_umask _((change_ty *));
void change_development_directory_clear _((change_ty *));
void change_integration_directory_clear _((change_ty *));
void change_architecture_clear _((change_ty *));
void change_architecture_add _((change_ty *, string_ty *));
void change_architecture_query _((change_ty *));
string_ty *change_architecture_name _((change_ty *, int));
cstate_architecture_times change_architecture_times_find _((change_ty *,
	string_ty *));
void change_build_time_set _((change_ty *));
void change_test_time_set _((change_ty *, time_t));
void change_test_baseline_time_set _((change_ty *, time_t));
void change_regression_test_time_set _((change_ty *));
void change_test_times_clear _((change_ty *));
void change_build_times_clear _((change_ty *));
void change_architecture_from_pconf _((change_ty *));

char *change_outstanding_builds _((change_ty *, time_t));
char *change_outstanding_tests _((change_ty *, time_t));
char *change_outstanding_tests_baseline _((change_ty *, time_t));
char *change_outstanding_tests_regression _((change_ty *, time_t));

int change_pathconf_name_max _((change_ty *));
string_ty *change_filename_check _((change_ty *, string_ty *, int));

void change_create_symlinks_to_baseline _((change_ty *, struct project_ty *, struct user_ty *, int));
void change_remove_symlinks_to_baseline _((change_ty *, struct project_ty *, struct user_ty *));

void change_rescind_test_exemption _((change_ty *));
string_ty *change_cstate_filename_get _((change_ty *));
string_ty *change_fstate_filename_get _((change_ty *));

void change_check_architectures _((change_ty *));
string_ty *change_new_test_filename_get _((change_ty *, long, int));
string_ty *change_development_directory_template _((change_ty *,
	struct user_ty *));

#endif /* LIBAEGIS_CHANGE_H */