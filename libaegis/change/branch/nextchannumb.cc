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

#include <common/skip_unlucky.h>
#include <common/trace.h>
#include <libaegis/change/branch.h>
#include <libaegis/interval.h>


long
change_branch_next_change_number(change::pointer cp, int is_a_change)
{
    cstate_ty       *cstate_data;
    cstate_branch_change_list_ty *lp;
    long            change_number;
    size_t          j, k;
    long            minchanum;
    int             reuse;

    trace(("change_branch_next_change_number(cp = %p)\n{\n", cp));
    cstate_data = cp->cstate_get();
    assert(cstate_data->branch);
    if (!cstate_data->branch->change)
    {
        cstate_data->branch->change =
            (cstate_branch_change_list_ty *)
            cstate_branch_change_list_type.alloc();
    }
    lp = cstate_data->branch->change;

    //
    // build an interval which contains the set of changes
    // (it could be empty)
    //
    interval ip1;
    for (j = 0; j < lp->length; )
    {
        for (k = j + 1; k < lp->length; ++k)
            if (lp->list[k - 1] + 1 != lp->list[k])
                break;
        ip1 += interval(lp->list[j], lp->list[k - 1]);
        j = k;
    }

    //
    // difference the above set from the interval [minchanum..max)
    // This avoids zero.  If the user wants a zero-numbered change,
    // she must ask for it.
    //
    if (is_a_change)
        minchanum = change_branch_minimum_change_number_get(cp);
    else
        minchanum = change_branch_minimum_branch_number_get(cp);
    interval ip2(minchanum, TRUNK_CHANGE_NUMBER - 1);
    interval ip3 = ip2 - ip1;
    assert(!ip3.empty());

    //
    // The result is the minimum acceptable number in the interval.
    // But !reuse means the first number in the last interval.
    //
    reuse = !is_a_change || change_branch_reuse_change_numbers_get(cp);
    if (!change_branch_skip_unlucky_get(cp))
    {
        if (reuse)
            change_number = ip3.first();
        else
            change_number = ip3.second_last();
    }
    else
    {
        for (;;)
        {
            if (reuse)
                change_number = ip3.first();
            else
                change_number = ip3.second_last();
            minchanum = skip_unlucky(change_number);
            if (minchanum == change_number)
                break;
            ip3 -= interval(change_number, minchanum - 1);
        }
    }
    trace(("return %ld;\n", change_number));
    trace(("}\n"));
    return change_number;
}


// vim: set ts=8 sw=4 et :
