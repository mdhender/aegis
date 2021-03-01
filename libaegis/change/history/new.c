/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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

#include <change.h>
#include <error.h> /* for assert */
#include <trace.h>
#include <user.h>


cstate_history
change_history_new(cp, up)
	change_ty	*cp;
	user_ty		*up;
{
	cstate		cstate_data;
	cstate_history	history_data;
	cstate_history	*history_data_p;
	type_ty		*type_p;

	trace(("change_history_new(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->history);
	history_data_p =
		cstate_history_list_type.list_parse
		(
			cstate_data->history,
			&type_p
		);
	assert(type_p == &cstate_history_type);
	history_data = cstate_history_type.alloc();
	*history_data_p = history_data;
	time(&history_data->when);
	history_data->who = str_copy(user_name(up));
	trace(("return %8.8lX;\n", history_data));
	trace((/*{*/"}\n"));
	return history_data;
}