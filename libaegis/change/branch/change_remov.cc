//
//      aegis - project change supervisor
//      Copyright (C) 2001-2008, 2012 Peter Miller
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


void
change_branch_change_remove(change::pointer cp, long change_number)
{
    cstate_ty       *cstate_data;
    cstate_branch_change_list_ty *lp;
    size_t          j;

    trace(("change_branch_change_remove(cp = %p, change_number = %ld)\n{\n",
        cp, change_number));
    cstate_data = cp->cstate_get();
    assert(cstate_data->branch);
    if (!cstate_data->branch->change)
    {
        cstate_data->branch->change =
            (cstate_branch_change_list_ty *)
            cstate_branch_change_list_type.alloc();
    }

    //
    // Remove the name from the list, if it is on the list.
    // Be conservative, look for duplicates.
    //
    lp = cstate_data->branch->change;
    for (j = 0; j < lp->length; ++j)
    {
        if (change_number == lp->list[j])
        {
            size_t          k;

            for (k = j + 1; k < lp->length; ++k)
                lp->list[k - 1] = lp->list[k];
            lp->length--;
            j--;
        }
    }

    if (cstate_data->branch->sub_branch)
    {
        cstate_branch_sub_branch_list_ty *lp2;

        //
        // Remove the name from the list, if it is on the list.
        // Be conservative, look for duplicates.
        //
        lp2 = cstate_data->branch->sub_branch;
        for (j = 0; j < lp2->length; ++j)
        {
            if (change_number == lp2->list[j])
            {
                size_t          k;

                for (k = j + 1; k < lp2->length; ++k)
                    lp2->list[k - 1] = lp2->list[k];
                lp2->length--;
                j--;
            }
        }
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
