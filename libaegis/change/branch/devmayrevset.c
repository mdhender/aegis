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
 * MANIFEST: functions to manipulate devmayrevsets
 */

#include <change/branch.h>
#include <error.h> /* for assert */
#include <trace.h>


void
change_branch_developer_may_review_set(cp, n)
	change_ty	*cp;
	int		n;
{
	cstate		cstate_data;
	cstate_branch	bp;

	trace(("change_branch_developer_may_review(cp = %8.8lX, n = %d)\n{\n"/*}*/,
		(long)cp, n));
	cstate_data = change_cstate_get(cp);
	bp = cstate_data->branch;
	assert(bp);
	bp->developer_may_review = n;
	trace((/*{*/"}\n"));
}
