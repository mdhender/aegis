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
 * MANIFEST: functions to manipulate nths
 */

#include <change/branch.h>
#include <error.h> /* for assert */
#include <trace.h>


string_ty *
change_branch_administrator_nth(change_ty *cp, long n)
{
	cstate		cstate_data;
	cstate_branch_administrator_list lp;
	string_ty	*result;

	trace(("change_branch_administrator_nth(cp = %8.8lX, n = %ld)\n{\n",
		(long)cp, n));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->administrator)
		cstate_data->branch->administrator =
			cstate_branch_administrator_list_type.alloc();
	lp = cstate_data->branch->administrator;

	if (n < 0 || n >= lp->length)
		result = 0;
	else
		result = lp->list[n];
	trace(("return %8.8lX;\n", (long)result));
	trace(("}\n"));
	return result;
}
