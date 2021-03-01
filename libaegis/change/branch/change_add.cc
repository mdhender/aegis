//
//	aegis - project change supervisor
//	Copyright (C) 2001-2008 Peter Miller
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
change_branch_change_add(change::pointer cp, long change_number,
    int is_a_branch)
{
    cstate_ty       *cstate_data;
    meta_type	    *type_p;
    long	    *addr;
    cstate_branch_change_list_ty *lp;
    size_t	    j;

    //
    // add it to the change list
    //
    trace(("change_branch_change_add(cp = %8.8lX, change_number = %ld)\n{\n",
	(long)cp, change_number));
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
    // make sure we don't have it already
    //
    for (j = 0; j < lp->length; ++j)
	if (change_number == lp->list[j])
    	    break;

    //
    // append it to the list
    //
    if (j >= lp->length)
    {
	addr =
    	    (long int *)
	    cstate_branch_change_list_type.list_parse
    	    (
       		cstate_data->branch->change,
       		&type_p
    	    );
	assert(type_p == &integer_type);
	*addr = change_number;
    }

    if (is_a_branch)
    {
	cstate_branch_sub_branch_list_ty *lp2;

	//
	// add it to the change list
	//
	if (!cstate_data->branch->sub_branch)
        {
	    cstate_data->branch->sub_branch =
                (cstate_branch_sub_branch_list_ty *)
	       	cstate_branch_sub_branch_list_type.alloc();
        }
	lp2 = cstate_data->branch->sub_branch;

	//
	// make sure we don't have it already
	//
	for (j = 0; j < lp2->length; ++j)
	    if (change_number == lp2->list[j])
	       	break;

	//
	// append it to the list
	//
	if (j >= lp2->length)
	{
	    addr =
	       	(long int *)
		cstate_branch_change_list_type.list_parse
	       	(
	    	    cstate_data->branch->sub_branch,
		    &type_p
	       	);
	    assert(type_p == &integer_type);
	    *addr = change_number;
	}
    }
    trace(("}\n"));
}
