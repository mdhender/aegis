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
// MANIFEST: functions to manipulate devmacrchgets
//

#include <libaegis/change/branch.h>
#include <common/error.h> // for assert


bool
change_branch_developers_may_create_changes_get(change_ty *cp)
{
    cstate_ty *cstate_data = change_cstate_get(cp);
    cstate_branch_ty *bp = cstate_data->branch;
    assert(bp);
    return bp->developers_may_create_changes;
}
