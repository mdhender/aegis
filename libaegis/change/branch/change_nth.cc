//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2003-2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/change/branch.h>


int
change_branch_change_nth(change::pointer cp, long n, long *cnp)
{
    cstate_ty       *cstate_data;
    cstate_branch_change_list_ty *lp;

    trace(("change_branch_change_nth(cp = %p, n = %ld)\n{\n", cp, n));
    cstate_data = cp->cstate_get();
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


// vim: set ts=8 sw=4 et :
