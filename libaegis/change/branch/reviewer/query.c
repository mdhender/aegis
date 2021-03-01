/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001-2003 Peter Miller;
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
 * MANIFEST: functions to manipulate querys
 */

#include <change/branch.h>
#include <error.h>
#include <trace.h>


int
change_branch_reviewer_query(change_ty *cp, string_ty *name)
{
	cstate		cstate_data;
	cstate_branch_reviewer_list lp;
	size_t		j;

	trace(("change_branch_reviewer_query(cp = %8.8lX, name = \"%s\")\n{\n",
		(long)cp, name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->reviewer)
		cstate_data->branch->reviewer =
			cstate_branch_reviewer_list_type.alloc();
	lp = cstate_data->branch->reviewer;
	for (j = 0; j < lp->length; ++j)
	{
		if (str_equal(lp->list[j], name))
		{
			trace(("return 1;\n"));
			trace(("}\n"));
			return 1;
		}
	}
	trace(("return 0;\n"));
	trace(("}\n"));
	return 0;
}
