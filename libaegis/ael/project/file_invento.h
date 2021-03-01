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

#ifndef LIBAEGIS_AEL_PROJECT_FILE_INVENTO_H
#define LIBAEGIS_AEL_PROJECT_FILE_INVENTO_H

class change_identifier; // forward
struct string_list_ty; // forward

void list_project_file_inventory(change_identifier &, string_list_ty *);

#endif // LIBAEGIS_AEL_PROJECT_FILE_INVENTO_H
