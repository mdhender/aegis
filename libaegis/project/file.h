/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995-1997, 2002, 2003 Peter Miller;
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
 * MANIFEST: interface definition for aegis/project/file/...c
 */

#ifndef AEGIS_PROJECT_FILE_H
#define AEGIS_PROJECT_FILE_H

#include <project.h>
#include <fstate.h>
#include <view_path.h>

/**
  * The project_file_find function is used to find the state information
  * of the named file within the project.  It will search the immediate
  * branch, and then any ancestor branches until the file is found.
  *
  * \param pp
  *	The project to search.
  * \param filename
  *	The base-relative name of the file to search for.
  * \param as_view_path
  *	If this is true, apply viewpath rules to the file (i.e. if
  *	it is removed, return a null pointer) if false return first
  *	instance found.
  */
fstate_src_ty *project_file_find(project_ty *pp, string_ty *filename,
    view_path_ty as_view_path);

string_ty *project_file_path(project_ty *, string_ty *);

/**
  * The project_file_find_fuzzy function is used to find the state
  * information for a project file when the project_file_find function
  * fails.  It uses fuzzy string matching, which is significantly slower
  * than exact searching, but can provide very useful error messages
  * for users.
  *
  * \param pp
  *	The project to search.
  * \param filename
  *	The base-relative name of the file to search for.
  * \param as_view_path
  *	If this is true, apply viewpath rules to the file (i.e. if
  *	it is removed, return a null pointer) if false return first
  *	instance found.
  */
fstate_src_ty *project_file_find_fuzzy(project_ty *pp, string_ty *filename,
    view_path_ty as_view_path);

void project_file_directory_query(project_ty *, string_ty *,
    struct string_list_ty *, struct string_list_ty *,
    view_path_ty as_view_path);
string_ty *project_file_directory_conflict(project_ty *, string_ty *);
fstate_src_ty *project_file_new(project_ty *, string_ty *);
void project_file_remove(project_ty *, string_ty *);

/**
  * The project_file_nth function is used to get the 'n'th file from
  * the list of project files.
  *
  * \param pp
  *	The project to search.
  * \param n
  *	The file number to obtain (zero based).
  * \param as_view_path
  *	If this is false, return all files; if true, ignore removed files.
  */
fstate_src_ty *project_file_nth(project_ty *pp, size_t n,
    view_path_ty as_view_path);

void project_search_path_get(project_ty *, struct string_list_ty *, int);
void project_file_shallow(project_ty *, string_ty *, long);
int project_file_shallow_check(project_ty *, string_ty *);
string_ty *project_file_version_path(project_ty *, fstate_src_ty *, int *);

#endif /* AEGIS_PROJECT_FILE_H */
