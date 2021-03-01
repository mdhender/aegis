/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate adds
 */

#include <change/branch.h>
#include <error.h>
#include <trace.h>


void
change_history_delta_name_add(cp, delta_number, delta_name)
    change_ty       *cp;
    long            delta_number;
    string_ty       *delta_name;
{
    cstate          cstate_data;
    cstate_branch_history_list h;
    size_t          j;
    cstate_branch_history he;

    trace(("change_history_delta_name_add(cp = %8.8lX, delta_number = %ld, "
	"delta_name = \"%s\")\n{\n", (long)cp, delta_number,
	delta_name->str_text));
    cstate_data = change_cstate_get(cp);
    assert(cstate_data->branch);
    h = cstate_data->branch->history;
    assert(h);
    assert(h->length);
    for (j = 0; j < h->length; ++j)
    {
	type_ty         *type_p;
	string_ty       **addr_p;

	he = h->list[j];
	if (he->delta_number != delta_number)
	    continue;

	/*
	 * add the name to the selected history entry
	 */
	if (!he->name)
	{
	    he->name = cstate_branch_history_name_list_type.alloc();
	}
	addr_p =
	    cstate_branch_history_name_list_type.list_parse(he->name, &type_p);
	assert(type_p == &string_type);
	*addr_p = str_copy(delta_name);
	break;
    }
    trace(("}\n"));
}
