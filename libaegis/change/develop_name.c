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
 * MANIFEST: functions to manipulate develop_names
 */

#include <change.h>
#include <error.h> /* for assert */
#include <trace.h>


string_ty *
change_developer_name(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	cstate_history	history_data;
	long		pos;

	trace(("change_developer_name(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->history);
	history_data = 0;
	for (pos = cstate_data->history->length - 1; pos >= 0 ; --pos)
	{
		history_data = cstate_data->history->list[pos];
		switch (history_data->what)
		{
		default:
			history_data = 0;
			continue;

		case cstate_history_what_develop_begin:
		case cstate_history_what_develop_begin_undo:
		case cstate_history_what_develop_end:
		case cstate_history_what_develop_end_undo:
			break;
		}
		break;
	}
	trace(("return \"%s\";\n",
		history_data ? history_data->who->str_text : ""));
	trace((/*{*/"}\n"));
	return (history_data ? history_data->who : 0);
}
