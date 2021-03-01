//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2006 Peter Miller
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
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface for libaegis/ael/project/details.cc
//

#ifndef LIBAEGIS_AEL_PROJECT_DETAILS_H
#define LIBAEGIS_AEL_PROJECT_DETAILS_H

struct string_ty; // forward
struct string_list_ty; // forward

/**
  * The list_change_details function is used to list the deatils of a change.
  *
  * @param project_name
  *     The name of the project the change is in.
  * @param change_number
  *     The number of the change to be listed.
  * @param arguments
  *     Additional listing arguments.
  */
void list_project_details(string_ty *project_name, long change_number,
    string_list_ty *arguments);

#endif // LIBAEGIS_AEL_PROJECT_DETAILS_H
