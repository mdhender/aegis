//
//	aegis - project change supervisor
//	Copyright (C) 2002-2007 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/change/branch.h>
#include <common/error.h> // for assert
#include <libaegis/project.h>


long
change_history_timestamp_to_delta(project_ty *pp, time_t when)
{
    cstate_ty       *cstate_data;
    cstate_branch_history_list_ty *hl;
    long            j;
    change::pointer cp;

    cp = pp->change_get();
    cstate_data = cp->cstate_get();
    if (!cstate_data->branch)
	return 0;
    hl = cstate_data->branch->history;
    if (!hl)
	return 0;
    for (j = hl->length - 1; j >= 0; --j)
    {
	cstate_branch_history_ty *bh;
	change::pointer cp2;
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
	    return bh->delta_number;
    }
    return 0;
}
