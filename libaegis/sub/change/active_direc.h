//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
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

#ifndef LIBAEGIS_SUB_CHANGE_ACTIVE_DIREC_H
#define LIBAEGIS_SUB_CHANGE_ACTIVE_DIREC_H

class sub_context_ty; // forward
class wstring; // forward
class wstring_list; // forward

/**
  * The sub_change_active_directory function implements the
  * $active_directory substitution.  The $active_directory substitution
  * is used to insert the absolute path of the change's development
  * directory or integration directory, depending om the state the
  * change is in.
  *
  * @param cntxt
  *     the substitution context
  * @param arg
  *     list of arguments, including the function name as [0]
  *
  * @returns
  *     a pointer to a string in dynamic memory; or NULL on error,
  *     setting suberr appropriately.
  */
wstring sub_change_active_directory(sub_context_ty *cntxt,
    const wstring_list &arg);

#endif // LIBAEGIS_SUB_CHANGE_ACTIVE_DIREC_H
