//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate reusechansets
//

#include <libaegis/change/branch.h>
#include <common/error.h>


void
change_branch_reuse_change_numbers_set(change_ty *cp, bool n)
{
    cstate_ty *cstate_data = change_cstate_get(cp);
    cstate_branch_ty *bp = cstate_data->branch;
    assert(bp);
    bp->reuse_change_numbers = n;
    bp->mask |= cstate_branch_reuse_change_numbers_mask;
}
