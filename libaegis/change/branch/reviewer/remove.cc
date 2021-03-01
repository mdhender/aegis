//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/change/branch.h>
#include <common/error.h>
#include <common/trace.h>


void
change_branch_reviewer_remove(change::pointer cp, string_ty *usrnam)
{
    cstate_ty       *cstate_data;
    cstate_branch_reviewer_list_ty *lp;
    size_t		j;

    trace(("change_branch_reviewer_remove(cp = %8.8lX, "
	"usrnam = \"%s\")\n{\n", (long)cp, usrnam->str_text));
    cstate_data = cp->cstate_get();
    assert(cstate_data->branch);
    if (!cstate_data->branch->reviewer)
    {
	cstate_data->branch->reviewer =
	    (cstate_branch_reviewer_list_ty *)
    	    cstate_branch_reviewer_list_type.alloc();
    }
    lp = cstate_data->branch->reviewer;

    //
    // Remove the name from the list, if it is on the list.
    // Be conservative, look for duplicates.
    //
    for (j = 0; j < lp->length; ++j)
    {
	if (str_equal(usrnam, lp->list[j]))
	{
    	    size_t		k;

    	    str_free(lp->list[j]);
    	    for (k = j + 1; k < lp->length; ++k)
       		lp->list[k - 1] = lp->list[k];
    	    lp->length--;
    	    j--;
	}
    }
    trace(("}\n"));
}
