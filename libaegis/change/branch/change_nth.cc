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
// MANIFEST: functions to manipulate change_nths
//

#include <change/branch.h>
#include <error.h>
#include <trace.h>


int
change_branch_change_nth(change_ty *cp, long n, long *cnp)
{
    cstate_ty       *cstate_data;
    cstate_branch_change_list_ty *lp;

    trace(("change_branch_change_nth(cp = %8.8lX, n = %ld)\n{\n", (long)cp, n));
    cstate_data = change_cstate_get(cp);
    assert(cstate_data->branch);
    if (!cstate_data->branch->change)
    {
	cstate_data->branch->change = (cstate_branch_change_list_ty *)
        cstate_branch_change_list_type.alloc();
    }
    lp = cstate_data->branch->change;

    if (n < 0 || (size_t)n >= lp->length)
    {
	trace(("return FALSE;\n"));
	trace(("}\n"));
	return 0;
    }
    assert(cnp);
    *cnp = lp->list[n];
    trace(("return %ld;\n", *cnp));
    trace(("}\n"));
    return 1;
}
