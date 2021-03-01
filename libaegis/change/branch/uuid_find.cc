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


void
change_branch_uuid_find(change_ty *cp, string_ty *uuid, change_list_ty &result)
{
    cstate_ty *cstate_data = change_cstate_get(cp);
    if (cstate_data->uuid && str_leading_prefix(cstate_data->uuid, uuid))
    {
	result.append(change_copy(cp));
	if (uuid->str_length == 36)
	    return;
    }
    if (cstate_data->branch)
    {
	project_ty *pp2 = project_bind_branch(cp->pp, cp);
	for (size_t j = 0; j < cstate_data->branch->change->length; ++j)
	{
	    long change_number = cstate_data->branch->change->list[j];
	    change_ty *cp2 = change_alloc(pp2, change_number);
	    change_bind_existing(cp);
	    change_branch_uuid_find(cp2, uuid, result);
	    if (uuid->str_length == 36 && result.size())
		return;
	    change_free(cp2);
	}
    }
}
