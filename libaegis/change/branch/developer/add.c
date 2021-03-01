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
 * MANIFEST: functions to manipulate adds
 */

#include <change/branch.h>
#include <error.h>
#include <trace.h>


void
change_branch_developer_add(change_ty *cp, string_ty *user_name)
{
    cstate_ty       *cstate_data;
    type_ty         *type_p;
    string_ty       **spp;
    cstate_branch_developer_list_ty *lp;
    size_t          j;

    trace(("change_branch_developer_add(cp = %8.8lX, user_name = \"%s\")\n{\n",
	(long)cp, user_name->str_text));
    cstate_data = change_cstate_get(cp);
    assert(cstate_data->branch);
    if (!cstate_data->branch->developer)
	cstate_data->branch->developer =
    	    cstate_branch_developer_list_type.alloc();
    lp = cstate_data->branch->developer;

    /*
     * make sure we don't have her already
     */
    for (j = 0; j < lp->length; ++j)
    {
	if (str_equal(user_name, lp->list[j]))
	{
    	    trace((/*{*/"}\n"));
    	    return;
	}
    }

    /*
     * append her to the list
     */
    spp =
	cstate_branch_developer_list_type.list_parse
	(
    	    cstate_data->branch->developer,
    	    &type_p
	);
    assert(type_p == &string_type);
    *spp = str_copy(user_name);
    trace(("}\n"));
}
