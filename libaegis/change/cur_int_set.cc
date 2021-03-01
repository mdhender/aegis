//
//	aegis - project change supervisor
//	Copyright (C) 2001-2008 Peter Miller
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


void
change_current_integration_set(change::pointer cp, long change_number)
{
    cstate_ty       *cstate_data;
    cstate_branch_ty *bp;

    trace(("change_current_integration_set(cp = %8.8lX, "
	"change_number = %ld)\n{\n", (long)cp, change_number));
    assert(change_number >= 0 || change_number == MAGIC_ZERO);
    cstate_data = cp->cstate_get();
    bp = cstate_data->branch;
    assert(bp);
    if (change_number)
    {
	assert(!(bp->mask & cstate_branch_currently_integrating_change_mask));
	bp->currently_integrating_change = change_number;
	bp->mask |= cstate_branch_currently_integrating_change_mask;
    }
    else
    {
	assert(bp->mask & cstate_branch_currently_integrating_change_mask);
	bp->currently_integrating_change = 0;
	bp->mask &= ~cstate_branch_currently_integrating_change_mask;
    }
    trace(("}\n"));
}
