//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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
#include <common/error.h>
#include <common/trace.h>


long
change_branch_next_delta_number(change::pointer cp)
{
    cstate_ty       *cstate_data;
    cstate_branch_ty *bp;
    long	    result;
    size_t	    j;

    trace(("change_branch_next_delta_number(cp = %8.8lX)\n{\n", (long)cp));
    cstate_data = cp->cstate_get();
    bp = cstate_data->branch;
    assert(bp);
    if (!bp || !bp->history || !bp->history->length)
    {
       	trace(("return 1;\n"));
       	trace(("}\n"));
       	return 1;
    }
    result = bp->history->length + 1;
    for (j = 0; j < bp->history->length; ++j)
    {
       	cstate_branch_history_ty *hp;

       	hp = bp->history->list[j];
       	if (hp->delta_number >= result)
	    result = hp->delta_number + 1;
    }
    trace(("return %ld;\n", result));
    trace(("}\n"));
    return result;
}
