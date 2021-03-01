/*
 *	aegis - project change supervisor
 *	Copyright (C) 1992-1999, 2001-2003 Peter Miller;
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
 * MANIFEST: interface definition for aegis/project.c
 */

#ifndef PROJECT_H
#define PROJECT_H

#include <ac/time.h>

#include <pstate.h>
#include <pattr.h>

struct string_list_ty; /* existence */
struct change_ty; /* existence */
struct sub_context_ty; /* existence */

typedef struct project_ty project_ty;
struct project_ty
{
    long            reference_count;
    string_ty       *name;
    string_ty       *home_path;
    string_ty       *baseline_path_unresolved;
    string_ty       *baseline_path;
    string_ty       *history_path;
    string_ty       *info_path;
    string_ty       *pstate_path;
    string_ty       *changes_path;
    pstate_ty       *pstate_data;
    int             is_a_new_file;
    long            lock_magic;
    struct change_ty *pcp;
    int             uid, gid;
    project_ty      *parent;
    long            parent_bn;
    struct string_list_ty *file_list[3];
    /*
     * if you add anything to this structure,
     * make sure you fix project_free in project.c
     * and zero-out the item in project_alloc
     */
};

project_ty *project_alloc(string_ty *name);
void project_bind_existing(project_ty *);
project_ty *project_bind_branch(project_ty *ppp, struct change_ty *bp);
void project_bind_new(project_ty *);
void project_list_get(struct string_list_ty *);
void project_list_inner(struct string_list_ty *, project_ty *);
project_ty *project_find_branch(project_ty *, const char *);
void project_free(project_ty *);
string_ty *project_name_get(project_ty *);
project_ty *project_copy(project_ty *);
struct change_ty *project_change_get(project_ty *);
string_ty *project_home_path_get(project_ty *);
string_ty *project_Home_path_get(project_ty *);
string_ty *project_top_path_get(project_ty *, int);
void project_home_path_set(project_ty *, string_ty *);
string_ty *project_baseline_path_get(project_ty *, int);
string_ty *project_history_path_get(project_ty *);
string_ty *project_info_path_get(project_ty *);
string_ty *project_changes_path_get(project_ty *);
string_ty *project_change_path_get(project_ty *, long);
string_ty *project_pstate_path_get(project_ty *);
pstate_ty *project_pstate_get(project_ty *);
void project_pstate_write(project_ty *);
void project_pstate_write_top(project_ty *);
void project_pstate_lock_prepare(project_ty *);
void project_pstate_lock_prepare_top(project_ty *);
void project_baseline_read_lock_prepare(project_ty *);
void project_baseline_write_lock_prepare(project_ty *);
void project_history_lock_prepare(project_ty *);
void project_error(project_ty *, struct sub_context_ty *, const char *);
void project_fatal(project_ty *, struct sub_context_ty *, const char *)
    NORETURN;
void project_verbose(project_ty *, struct sub_context_ty *, const char *);
void project_change_append(project_ty *, long, int);
void project_change_delete(project_ty *, long);
int project_change_number_in_use(project_ty *, long);
string_ty *project_version_short_get(project_ty *);
string_ty *project_version_get(project_ty *);
int project_uid_get(project_ty *);
int project_gid_get(project_ty *);
struct user_ty *project_user(project_ty *);
void project_become(project_ty *);
void project_become_undo(void);
long project_next_test_number_get(project_ty *);
int project_is_readable(project_ty *);
long project_minimum_change_number_get(project_ty *);
void project_minimum_change_number_set(project_ty *, long);
int project_reuse_change_numbers_get(project_ty *);
void project_reuse_change_numbers_set(project_ty *, int);
long project_minimum_branch_number_get(project_ty *);
void project_minimum_branch_number_set(project_ty *, long);
int project_skip_unlucky_get(project_ty *);
void project_skip_unlucky_set(project_ty *, int);
int project_compress_database_get(project_ty *);
void project_compress_database_set(project_ty *, int);
int project_develop_end_action_get(project_ty *);
void project_develop_end_action_set(project_ty *, int);
int project_protect_development_directory_get(project_ty *);
void project_protect_development_directory_set(project_ty *, int);

int break_up_version_string(const char *, long *, int, int *, int);
void extract_version_from_project_name(string_ty **, long *, int, int *);
int project_name_ok(string_ty *);

struct pconf_ty *project_pconf_get(project_ty *);

project_ty *project_new_branch(project_ty *, struct user_ty *, long,
    string_ty *);
void project_file_list_invalidate(project_ty *);

#endif /* PROJECT_H */
