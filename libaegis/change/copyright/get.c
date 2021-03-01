/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate gets
 */

#include <ac/stdlib.h>

#include <change/branch.h>
#include <error.h>
#include <project.h>


static void
change_copyright_years_slurp(change_ty *cp, int *a, int amax, int *alen_p)
{
    int             j, k;
    int             n;
    cstate_ty       *cstate_data;
    cstate_copyright_years_list_ty *cylp;

    cstate_data = change_cstate_get(cp);
    cylp = cstate_data->copyright_years;
    if (!cylp)
	return;
    for (j = 0; j < cylp->length; ++j)
    {
	if (*alen_p >= amax)
	    return;
	n = cylp->list[j];
	for (k = 0; k < *alen_p; ++k)
	    if (a[k] == n)
	       	break;
	if (k >= *alen_p)
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

    a = va;
    b = vb;
    return (*a - *b);
}


void
change_copyright_years_get(change_ty *cp, int *a, int amax, int *alen_p)
{
    project_ty      *pp;

    /*
     * Get the years specific to this change.
     */
    assert(alen_p);
    *alen_p = 0;
    change_copyright_years_slurp(cp, a, amax, alen_p);

    /*
     * Walk up the list of ancestors until we get to the trunk
     * extracting the years specific to each branch.
     */
    for (pp = cp->pp; pp; pp = pp->parent)
    {
	change_copyright_years_slurp
	(
	    project_change_get(pp),
	    a,
	    amax,
	    alen_p
	);
    }

    /*
     * sort the years into ascending order
     */
    if (*alen_p >= 2)
	qsort(a, *alen_p, sizeof(a[0]), change_copyright_years_cmp);
}
