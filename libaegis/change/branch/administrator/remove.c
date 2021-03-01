/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate removes
 */

#include <change/branch.h>
#include <error.h>
#include <trace.h>


void
change_branch_administrator_remove(cp, user_name)
	change_ty	*cp;
	string_ty	*user_name;
{
	cstate		cstate_data;
	cstate_branch_administrator_list lp;
	size_t		j;

	trace(("change_branch_administrator_remove(cp = %8.8lX, user_name = \
\"%s\")\n{\n"/*}*/, (long)cp, user_name->str_text));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->administrator)
		cstate_data->branch->administrator =
			cstate_branch_administrator_list_type.alloc();
	lp = cstate_data->branch->administrator;

	/*
	 * Remove the name from the list, if it is on the list.
	 * Be conservative, look for duplicates.
	 */
	for (j = 0; j < lp->length; ++j)
	{
		if (str_equal(user_name, lp->list[j]))
		{
			size_t		k;

			str_free(lp->list[j]);
			for (k = j + 1; k < lp->length; ++k)
				lp->list[k - 1] = lp->list[k];
			lp->length--;
			j--;
		}
	}
	trace((/*{*/"}\n"));
}
