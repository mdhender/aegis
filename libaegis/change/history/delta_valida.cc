//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate delta_validas
//

#include <change/branch.h>


int
change_history_delta_validate(change_ty *cp, long delta_number)
{
    cstate_ty       *cstate_data;
    cstate_branch_history_list_ty *h;
    size_t          j;

    cstate_data = change_cstate_get(cp);
    if (!cstate_data->branch)
	return 0;
    h = cstate_data->branch->history;
    if (!h)
	return 0;
    for (j = 0; j < h->length; ++j)
    {
	if (h->list[j]->delta_number == delta_number)
    	    return 1;
    }
    return 0;
}
