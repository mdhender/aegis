/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate cur_int_sets
 */

#include <change/branch.h>
#include <error.h>
#include <trace.h>


void
change_current_integration_set(change_ty *cp, long change_number)
{
    cstate	    cstate_data;
    cstate_branch   bp;

    trace(("change_current_integration_set(cp = %8.8lX, "
	"change_number = %ld)\n{\n", (long)cp, change_number));
    assert(change_number >= 0 || change_number == MAGIC_ZERO);
    cstate_data = change_cstate_get(cp);
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
    trace((/*{*/"}\n"));
}
