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
#include <common/error.h> // for assert
#include <common/trace.h>


int
change_branch_change_number_in_use(change::pointer cp, long cn)
{
    int             result;
    cstate_ty       *cstate_data;
    cstate_branch_change_list_ty *lp;
    size_t          j;

    trace(("change_branch_change_number_in_use(cp = %8.8lX, cn = %ld)\n{\n",
	(long)cp, cn));
    result = 0;
    cstate_data = cp->cstate_get();
    assert(cstate_data->branch);
    lp = cstate_data->branch->change;
    if (lp)
    {
	for (j = 0; j < lp->length; ++j)
	{
	    if (lp->list[j] == cn)
	    {
	       	result = 1;
	       	break;
	    }
	}
    }
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}
