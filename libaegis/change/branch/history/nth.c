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
 * MANIFEST: functions to manipulate nths
 */

#include <change/branch.h>
#include <error.h>
#include <str_list.h>
#include <trace.h>


int
change_branch_history_nth(change_ty *cp, long n, long *cnp, long *dnp,
    string_list_ty *name)
{
    cstate_ty       *cstate_data;
    cstate_branch_history_list_ty *lp;
    int             result;

    trace(("change_branch_history_nth(cp = %8.8lX, n = %ld)\n{\n",
	(long)cp, n));
    cstate_data = change_cstate_get(cp);
    assert(cstate_data->branch);
    if (!cstate_data->branch->history)
	cstate_data->branch->history =
    	    cstate_branch_history_list_type.alloc();
    lp = cstate_data->branch->history;
    if (n < 0 || n >= lp->length)
	result = 0;
    else
    {
	cstate_branch_history_ty * hp;

	hp = lp->list[n];
	*cnp = hp->change_number;
	*dnp = hp->delta_number;
	string_list_constructor(name);
	if (hp->name)
	{
	    size_t          j;

	    for (j = 0; j < hp->name->length; ++j)
	       	string_list_append(name, hp->name->list[j]);
	}
	result = 1;
    }
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}
