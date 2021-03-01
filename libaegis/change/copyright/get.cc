//
// aegis - project change supervisor
// Copyright (C) 2001, 2003-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/stdlib.h>

#include <libaegis/change/branch.h>
#include <libaegis/project.h>


void
change_copyright_years_slurp(change::pointer cp, int *a, int amax, int *alen_p)
{
    size_t          j, k;
    int             n;
    cstate_ty       *cstate_data;
    cstate_copyright_years_list_ty *cylp;

    cstate_data = cp->cstate_get();
    cylp = cstate_data->copyright_years;
    if (!cylp)
        return;
    for (j = 0; j < cylp->length; ++j)
    {
        if (*alen_p >= amax)
            return;
        n = (int)cylp->list[j];
        for (k = 0; k < (size_t)*alen_p; ++k)
            if (a[k] == n)
                break;
        if (k >= (size_t)*alen_p)
        {
            a[*alen_p] = n;
            ++*alen_p;
        }
    }
}


static int
change_copyright_years_cmp(const void *va, const void *vb)
{
    const int       *a;
    const int       *b;

    a = (const int *)va;
    b = (const int *)vb;
    return (*a - *b);
}


void
change_copyright_years_get(change::pointer cp, int *a, int amax, int *alen_p)
{
    //
    // Get the years specific to this change.
    //
    assert(alen_p);
    *alen_p = 0;
    change_copyright_years_slurp(cp, a, amax, alen_p);

    //
    // Walk up the list of ancestors until we get to the trunk
    // extracting the years specific to each branch.
    //
    cp->pp->copyright_years_slurp(a, amax, alen_p);

    //
    // sort the years into ascending order
    //
    if (*alen_p >= 2)
        qsort(a, *alen_p, sizeof(a[0]), change_copyright_years_cmp);
}


// vim: set ts=8 sw=4 et :
