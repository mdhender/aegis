//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
#include <project.h>


change_ty *
change_branch_uuid_find(change_ty *cp, string_ty *uuid)
{
    cstate_ty       *cstate_data;

    cstate_data = change_cstate_get(cp);
    if (cstate_data->uuid && str_equal(uuid, cstate_data->uuid))
	return cp;
    if (cstate_data->branch)
    {
	project_ty      *pp2;
	size_t          j;

	pp2 = project_bind_branch(cp->pp, cp);
	for (j = 0; j < cstate_data->branch->change->length; ++j)
	{
	    long            change_number;
	    change_ty       *cp2;
	    change_ty       *result;

	    change_number = cstate_data->branch->change->list[j];
	    cp2 = change_alloc(pp2, change_number);
	    change_bind_existing(cp);
	    result = change_branch_uuid_find(cp2, uuid);
	    if (result)
		return result;
	    change_free(cp2);
	}
    }
    return 0;
}
