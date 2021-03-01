//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2008 Peter Miller
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


bool
change_history_delta_validate(change::pointer cp, long delta_number)
{
    cstate_ty       *cstate_data;
    cstate_branch_history_list_ty *h;
    size_t          j;

    cstate_data = cp->cstate_get();
    if (!cstate_data->branch)
	return false;
    h = cstate_data->branch->history;
    if (!h)
	return false;
    for (j = 0; j < h->length; ++j)
    {
	if (h->list[j]->delta_number == delta_number)
    	    return true;
    }
    return false;
}
