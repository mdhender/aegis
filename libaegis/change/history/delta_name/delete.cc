//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate deletes
//

#include <libaegis/change/branch.h>
#include <common/error.h>
#include <common/trace.h>


void
change_history_delta_name_delete(change_ty *cp, string_ty *delta_name)
{
    cstate_ty       *cstate_data;
    cstate_branch_history_list_ty *h;
    size_t          j;

    trace(("change_history_delta_name_delete(cp = %8.8lX, "
	"delta_name = \"%s\")\n{\n", (long)cp, delta_name->str_text));
    cstate_data = change_cstate_get(cp);
    assert(cstate_data->branch);
    h = cstate_data->branch->history;
    assert(h);
    assert(h->length);
    for (j = 0; j < h->length; ++j)
    {
	cstate_branch_history_ty *he;
	cstate_branch_history_name_list_ty *nlp;
	size_t		k, m;

	he = h->list[j];
	nlp = he->name;
	if (!nlp || !nlp->length)
	    continue;
	for (k = 0; k < nlp->length; ++k)
	{
	    if (!str_equal(nlp->list[k], delta_name))
		continue;

	    //
	    // remove the name from the list
	    //
	    str_free(nlp->list[k]);
	    for (m = k + 1; m < nlp->length; ++m)
		nlp->list[m - 1] = nlp->list[m];
	    k--;

	    nlp->length--;
	    if (nlp->length == 0)
	    {
		he->name = 0;
		cstate_branch_history_name_list_type.free(nlp);
		break;
	    }
	}
    }
    trace(("}\n"));
}
