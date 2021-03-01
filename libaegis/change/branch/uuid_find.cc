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

#include <libaegis/change/branch.h>
#include <libaegis/change/list.h>
#include <libaegis/project.h>


void
change_branch_uuid_find(change::pointer cp, string_ty *uuid,
    change_list_ty &result)
{
    cstate_ty *cstate_data = cp->cstate_get();
    if (cstate_data->uuid && str_leading_prefix(cstate_data->uuid, uuid))
    {
	result.append(change_copy(cp));
	if (uuid->str_length == 36)
	    return;
    }
    if (cstate_data->branch)
    {
	project_ty *pp2 = cp->pp->bind_branch(cp);
	for (size_t j = 0; j < cstate_data->branch->change->length; ++j)
	{
	    long change_number = cstate_data->branch->change->list[j];
	    change::pointer cp2 = change_alloc(pp2, change_number);
	    change_bind_existing(cp);
	    change_branch_uuid_find(cp2, uuid, result);
	    if (uuid->str_length == 36 && result.size())
		return;
	    change_free(cp2);
	}
    }
}
