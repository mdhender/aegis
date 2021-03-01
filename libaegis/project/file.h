//
// aegis - project change supervisor
// Copyright (C) 1995-1997, 2002, 2003, 2005-2008, 2011, 2012 Peter Miller
// Copyright (C) 2006 Walter Franzini
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AEGIS_PROJECT_FILE_H
#define AEGIS_PROJECT_FILE_H

#include <libaegis/fstate.fmtgen.h>
#include <libaegis/project.h>
#include <libaegis/view_path.h>

struct cstate_src_ty; // forward

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
string_ty *project_file_path(project *pp, string_ty *file_name);

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
string_ty *project_file_path(project *pp, fstate_src_ty *src);

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
string_ty *project_file_path(project *pp, cstate_src_ty *src);

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
string_ty *project_file_path_by_uuid(project *pp, string_ty *uuid);

string_ty *project_file_directory_conflict(project *, string_ty *);

void project_file_remove(project *, string_ty *);

void project_file_shallow(project *, string_ty *, long);
int project_file_shallow_check(project *, string_ty *);
string_ty *project_file_version_path(project *, fstate_src_ty *, int *);

#endif // AEGIS_PROJECT_FILE_H
// vim: set ts=8 sw=4 et :
