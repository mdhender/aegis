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
// MANIFEST: functions to manipulate branlistgets
//

#include <libaegis/change/branch.h>
#include <common/error.h>
#include <common/trace.h>


void
change_branch_sub_branch_list_get(change_ty *cp, long **list, size_t *len)
{
    cstate_ty       *cstate_data;
    cstate_branch_sub_branch_list_ty *lp;

    trace(("change_branch_list_get(cp = %8.8lX)\n{\n", (long)cp));
    cstate_data = change_cstate_get(cp);
    assert(cstate_data->branch);
    lp = cstate_data->branch->sub_branch;
    if (!lp)
    {
	*list = 0;
	*len = 0;
    }
    else
    {
	*list = lp->list;
	*len = lp->length;
    }
    trace(("}\n"));
}
