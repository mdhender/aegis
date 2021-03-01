//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate time2changes
//

#include <change/branch.h>
#include <error.h> // for assert
#include <project.h>


long
change_history_change_by_timestamp(project_ty *pp, time_t when)
{
    cstate_ty       *cstate_data;
    cstate_branch_history_list_ty *hl;
    long            j;
    change_ty       *cp;

    cp = project_change_get(pp);
    cstate_data = change_cstate_get(cp);
    if (!cstate_data->branch)
	return 0;
    hl = cstate_data->branch->history;
    if (!hl)
	return 0;
    for (j = hl->length - 1; j >= 0; --j)
    {
	cstate_branch_history_ty *bh;
	change_ty	*cp2;
	time_t		result;

	bh = hl->list[j];
	assert(bh);
	if (!bh)
	    continue;
	cp2 = change_alloc(pp, bh->change_number);
	change_bind_existing(cp2);
	result = change_completion_timestamp(cp2);
	change_free(cp2);
	if (result <= when)
	    return bh->change_number;
    }
    return 0;
}
