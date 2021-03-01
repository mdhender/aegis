/*
 *	aegis - project change supervisor
 *	Copyright (C) 1992, 1993, 1994 Peter Miller;
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: interface definition for aegis/project.c
 */

#ifndef PROJECT_H
#define PROJECT_H

#include <pstate.h>
#include <pattr.h>

struct wlist;

typedef struct project_ty project_ty;
struct project_ty
{
	long		reference_count;
	string_ty	*name;
	string_ty	*home_path;
	string_ty	*baseline_path_unresolved;
	string_ty	*baseline_path;
	string_ty	*history_path;
	string_ty	*info_path;
	string_ty	*pstate_path;
	string_ty	*changes_path;
	pstate		pstate_data;
	int		is_a_new_file;
	long		lock_magic;
	/*
	 * if you add anything to this structure,
	 * make sure you fix project_free in project.c
	 */
};

project_ty *project_alloc _((string_ty *name));
void project_bind_existing _((project_ty *));
void project_bind_new _((project_ty *));
void project_free _((project_ty *));
string_ty *project_name_get _((project_ty *));
project_ty *project_copy _((project_ty *));
string_ty *project_home_path_get _((project_ty *));
void project_home_path_set _((project_ty *, string_ty *));
string_ty *project_baseline_path_get _((project_ty *, int));
string_ty *project_history_path_get _((project_ty *));
string_ty *project_info_path_get _((project_ty *));
string_ty *project_changes_path_get _((project_ty *));
string_ty *project_change_path_get _((project_ty *, long));
string_ty *project_pstate_path_get _((project_ty *));
pstate project_pstate_get _((project_ty *));
void project_pstate_write _((project_ty *));
void project_pstate_lock_prepare _((project_ty *));
void project_build_read_lock_prepare _((project_ty *));
void project_build_write_lock_prepare _((project_ty *));
pattr project_pattr_get _((project_ty *));
void project_pattr_set _((project_ty *, pattr));
pstate_src project_src_find _((project_ty *, string_ty *));
pstate_src project_src_find_fuzzy _((project_ty *, string_ty *));
int project_src_dir _((project_ty *, string_ty *, struct wlist *));
pstate_src project_src_new _((project_ty *, string_ty *));
void project_src_remove _((project_ty *, string_ty *));
int project_administrator_query _((project_ty *, string_ty *));
void project_administrator_add _((project_ty *, string_ty *));
void project_administrator_delete _((project_ty *, string_ty *));
int project_integrator_query _((project_ty *, string_ty *));
void project_integrator_add _((project_ty *, string_ty *));
void project_integrator_delete _((project_ty *, string_ty *));
int project_reviewer_query _((project_ty *, string_ty *));
void project_reviewer_add _((project_ty *, string_ty *));
void project_reviewer_delete _((project_ty *, string_ty *));
int project_developer_query _((project_ty *, string_ty *));
void project_developer_add _((project_ty *, string_ty *));
void project_developer_delete _((project_ty *, string_ty *));
pstate_history project_history_new _((project_ty *));
long project_last_change_integrated _((project_ty *));
void project_error _((project_ty *, char *, ...));
void project_fatal _((project_ty *, char *, ...));
void project_verbose _((project_ty *, char *, ...));
void project_change_append _((project_ty *, long));
void project_change_delete _((project_ty *, long));
string_ty *project_version_get _((project_ty *));
string_ty *project_owner _((project_ty *));
string_ty *project_group _((project_ty *));
string_ty *project_default_development_directory _((project_ty *));
struct user_ty *project_user _((project_ty *));
void project_become _((project_ty *));
void project_become_undo _((void));
int project_umask _((project_ty *));
int project_delta_exists _((project_ty *pp, long dn));
long project_delta_name_to_number _((project_ty *pp, char *dn));
string_ty *project_delta_to_edit _((project_ty *pp, long delta, string_ty *fn));

#endif /* PROJECT_H */
