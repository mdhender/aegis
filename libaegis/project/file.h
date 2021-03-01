//
//	aegis - project change supervisor
//	Copyright (C) 1995-1997, 2002, 2003, 2005-2008 Peter Miller
//	Copyright (C) 2006 Walter Franzini
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

#ifndef AEGIS_PROJECT_FILE_H
#define AEGIS_PROJECT_FILE_H

#include <libaegis/fstate.h>
#include <libaegis/project.h>
#include <libaegis/view_path.h>

struct cstate_src_ty; // forward

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

/**
  * The project_file_find_by_uuid function is used to find the state
  * information of a file within the project, given the file's UUID.  It
  * will search the immediate branch, and then any ancestor branches
  * until the file is found.
  *
  * \param pp
  *	The project to search.
  * \param uuid
  *	The UUID of the file to search for.
  * \param vp
  *	If this is true, apply viewpath rules to the file (i.e. if
  *	it is removed, return a null pointer) if false return first
  *	instance found.
  */
inline DEPRECATED fstate_src_ty *
project_file_find_by_uuid(project_ty *pp, string_ty *uuid, view_path_ty vp)
{
    return pp->file_find_by_uuid(uuid, vp);
}

/**
  * The project_file_find function is used to find the state
  * information of a file within the project, given the corresponding
  * change file's meta-data.  It will search the immediate branch, and
  * then any ancestor branches until the file is found.
  *
  * \param pp
  *	The project to search.
  * \param c_src
  *     The change file meta-data for which the corresponding project
  *     file is sought.
  * \param as_view_path
  *	If this is true, apply viewpath rules to the file (i.e. if
  *	it is removed, return a null pointer) if false return first
  *	instance found.
  */
fstate_src_ty *project_file_find(project_ty *pp, fstate_src_ty *c_src,
    view_path_ty as_view_path);

/**
  * The project_file_find function is used to find the state information
  * of a file within the project, given the corresponding change file's
  * meta-data.  It will search the immediate branch, and then any
  * ancestor branches until the file is found.
  *
  * \param pp
  *	The project to search.
  * \param c_src
  *     The change file meta-data for which the corresponding project
  *     file is sought.
  * \param as_view_path
  *	If this is true, apply viewpath rules to the file (i.e. if
  *	it is removed, return a null pointer) if false return first
  *	instance found.
  */
fstate_src_ty *project_file_find(project_ty *pp, cstate_src_ty *c_src,
    view_path_ty as_view_path);

/**
  * The project_file_path function is used to obtain the absolute path
  * to the given project file.
  *
  * @param pp
  *     The project in question.
  * @param file_name
  *     The name of the file in question.
  * @returns
  *     string containing absolute path of file
  * @note
  *     It is a bug to callthis function for a file which does not
  *     exist, or is not a project source file, or is a removed source
  *     file.
  */
string_ty *project_file_path(project_ty *pp, string_ty *file_name);

/**
  * The project_file_path function is used to obtain the absolute path
  * to the given project file.
  *
  * @param pp
  *     The project in question.
  * @param src
  *     The file in question.
  * @returns
  *     string containing absolute path of file
  * @note
  *     It is a bug to callthis function for a file which does not
  *     exist, or is not a project source file, or is a removed source
  *     file.
  */
string_ty *project_file_path(project_ty *pp, fstate_src_ty *src);

/**
  * The project_file_path function is used to obtain the absolute path
  * to the given project file.
  *
  * @param pp
  *     The project in question.
  * @param src
  *     The file in question.
  * @returns
  *     string containing absolute path of file
  * @note
  *     It is a bug to callthis function for a file which does not
  *     exist, or is not a project source file, or is a removed source
  *     file.
  */
string_ty *project_file_path(project_ty *pp, cstate_src_ty *src);

/**
  * The project_file_path_by_uuid function is used to obtain the
  * absolute path to a project file specified by its UUID.
  *
  * @param pp
  *     The project in question.
  * @param uuid
  *     The UUID of the file in question.
  * @returns
  *     string containing absolute path of file
  * @note
  *     It is a bug to callthis function for a file which does not
  *     exist, or is not a project source file, or is a removed source
  *     file.
  */
string_ty *project_file_path_by_uuid(project_ty *pp, string_ty *src);

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
inline DEPRECATED fstate_src_ty *
project_file_find_fuzzy(project_ty *pp, string_ty *filename,
    view_path_ty as_view_path)
{
    return pp->file_find_fuzzy(filename, as_view_path);
}

void project_file_directory_query(project_ty *, string_ty *,
    struct string_list_ty *, struct string_list_ty *,
    view_path_ty as_view_path);
string_ty *project_file_directory_conflict(project_ty *, string_ty *);

inline DEPRECATED fstate_src_ty *
project_file_new(project_ty *pp, string_ty *fn)
{
    return pp->file_new(fn);
}

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
inline DEPRECATED fstate_src_ty *
project_file_nth(project_ty *pp, size_t n, view_path_ty as_view_path)
{
    return pp->file_nth(n, as_view_path);
}

void project_search_path_get(project_ty *, struct string_list_ty *, int);
void project_file_shallow(project_ty *, string_ty *, long);
int project_file_shallow_check(project_ty *, string_ty *);
string_ty *project_file_version_path(project_ty *, fstate_src_ty *, int *);

#endif // AEGIS_PROJECT_FILE_H
