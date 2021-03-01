//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2003, 2005-2007 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
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
// MANIFEST: interface definition for libaegis/ael/change/files.c
//

#ifndef LIBAEGIS_AEL_CHANGE_FILES_H
#define LIBAEGIS_AEL_CHANGE_FILES_H

#include <libaegis/change.h>

struct string_ty; // existence
struct string_list_ty; // existence

/**
  * The list_change_file function is used to list the files in a change set.
  *
  * @param project_name
  *     The name of th eproject (or branch) the change is within
  * @param change_number
  *     The change number within the project
  * @param args
  *     additional command line arguments
  */
void list_change_files(string_ty *project_name, long change_number,
    string_list_ty *args);

/**
  * The list_change_file function is used to list the files in a change set.
  *
  * @param cp
  *     The change of interest
  * @param args
  *     additional command line arguments
  */
void list_change_files(change::pointer cp, string_list_ty *args);

#endif // LIBAEGIS_AEL_CHANGE_FILES_H
