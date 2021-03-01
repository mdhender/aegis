//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#ifndef LIBAEGIS_CHANGE_FILE_LIST_GET_H
#define LIBAEGIS_CHANGE_FILE_LIST_GET_H

#include <libaegis/change/file.h>


/**
  * The project_file_list_get function is used to obtain a list of file names.
  *
  * @param cp
  *     The change to be scanned.
  * @param as_view_path
  *	How to interpret the search path from development directory,
  *     then branch baseline, and so on until the trunk baseline.
  */
struct string_list_ty *change_file_list_get(change::pointer cp,
    view_path_ty as_view_path);

#endif // LIBAEGIS_CHANGE_FILE_LIST_GET_H
