//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2008 Peter Miller
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

#include <libaegis/change.h>
#include <common/error.h> // for assert
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
change_bind_existing(change::pointer cp)
{
    change::pointer pcp;
    cstate_ty       *pcsp;
    cstate_branch_change_list_ty *lp;
    size_t          j;

    //
    // verify the change number given on the command line
    //
    trace(("change_bind_existing(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    if (cp->number == TRUNK_CHANGE_NUMBER)
    {
	trace(("}\n"));
	return;
    }
    pcp = cp->pp->change_get();
    pcsp = pcp->cstate_get();
    if (!pcsp->branch)
    {
	assert(0);
	pcsp->branch = (cstate_branch_ty *)cstate_branch_type.alloc();
    }
    if (!pcsp->branch->change)
    {
	pcsp->branch->change =
	    (cstate_branch_change_list_ty *)
            cstate_branch_change_list_type.alloc();
    }
    lp = pcsp->branch->change;
    for (j = 0; j < lp->length; ++j)
    {
	if (lp->list[j] == cp->number)
    	    break;
    }
    if (j >= lp->length)
	change_fatal(cp, 0, i18n("unknown change"));
    trace(("}\n"));
}


int
change_bind_existing_errok(change::pointer cp)
{
    change::pointer pcp;
    cstate_ty       *pcsp;
    cstate_branch_change_list_ty *lp;
    size_t          j;

    //
    // verify the change number given on the command line
    //
    trace(("change_bind_existing_errok(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    if (cp->number == TRUNK_CHANGE_NUMBER)
    {
	trace(("return 1;\n"));
	trace(("}\n"));
	return 1;
    }
    pcp = cp->pp->change_get();
    pcsp = pcp->cstate_get();
    if (!pcsp->branch)
    {
	assert(0);
	pcsp->branch = (cstate_branch_ty *)cstate_branch_type.alloc();
    }
    if (!pcsp->branch->change)
	pcsp->branch->change =
	    (cstate_branch_change_list_ty *)
	    cstate_branch_change_list_type.alloc();
    lp = pcsp->branch->change;
    for (j = 0; j < lp->length; ++j)
    {
	if (lp->list[j] == cp->number)
    	    break;
    }
    if (j >= lp->length)
    {
	trace(("return 0;\n"));
	trace(("}\n"));
	return 0;
    }
    trace(("return 1;\n"));
    trace(("}\n"));
    return 1;
}
