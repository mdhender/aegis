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
 * MANIFEST: functions to manipulate news
 */

#include <change/branch.h>
#include <error.h>
#include <trace.h>


void
change_branch_history_new(change_ty *cp, long delta_number, long change_number)
{
	cstate		cstate_data;
	cstate_branch_history hp;
	cstate_branch_history *hpp;
	type_ty		*type_p;

	trace(("change_branch_history_new(cp = %8.8lX, delta_number = %ld, \
change_number = %ld)\n{\n"/*}*/, (long)cp, delta_number, change_number));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->history)
		cstate_data->branch->history =
			cstate_branch_history_list_type.alloc();
	hpp =
		cstate_branch_history_list_type.list_parse
		(
			cstate_data->branch->history,
			&type_p
		);
	assert(type_p == &cstate_branch_history_type);
	hp = cstate_branch_history_type.alloc();
	*hpp = hp;
	hp->delta_number = delta_number;
	hp->change_number = change_number;
	trace((/*{*/"}\n"));
}
