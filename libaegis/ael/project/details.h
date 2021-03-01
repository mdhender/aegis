//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2006-2008 Peter Miller
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

#ifndef LIBAEGIS_AEL_PROJECT_DETAILS_H
#define LIBAEGIS_AEL_PROJECT_DETAILS_H

struct string_ty; // forward
struct string_list_ty; // forward

/**
  * The list_change_details function is used to list the deatils of a change.
  *
  * @param cid
  *     The change to be listed.
  * @param arguments
  *     Additional listing arguments.
  */
void list_project_details(change_identifier &cid, string_list_ty *arguments);

#endif // LIBAEGIS_AEL_PROJECT_DETAILS_H
