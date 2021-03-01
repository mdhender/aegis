//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
//	All rights reserved.
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
// MANIFEST: functions to manipulate uuid_finds
//

#include <change/branch.h>
#include <change/list.h>
#include <project.h>


change_ty *
project_uuid_find(project_ty *pp, string_ty *uuid)
{
    while (pp->parent)
	pp = pp->parent;
    change_list_ty result;
    change_branch_uuid_find(project_change_get(pp), uuid, result);
    if (result.size() == 1)
	return result[0];
    for (size_t j = 0; j < result.size(); ++j)
	 change_free(result[j]);
    return 0;
}
