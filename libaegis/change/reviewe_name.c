/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate reviewe_names
 */

#include <change.h>
#include <error.h> /* for assert */
#include <trace.h>


string_ty *
change_reviewer_name(change_ty *cp)
{
	cstate		cstate_data;
	cstate_history	history_data;
	long		pos;

	trace(("change_reviewer_name(cp = %08lX)\n{\n"/*}*/, (long)cp));
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

		case cstate_history_what_develop_end_2ai:
			/*
			 * This is a special case.  It is as if the
			 * developer has reviewed her own change.
			 * So we fake it.
			 */
			break;

		case cstate_history_what_review_begin:
		case cstate_history_what_review_begin_undo:
		case cstate_history_what_review_fail:
		case cstate_history_what_review_pass:
		case cstate_history_what_review_pass_undo:
			break;
		}
		break;
	}
	trace(("return \"%s\";\n",
		history_data ? history_data->who->str_text : ""));
	trace((/*{*/"}\n"));
	return (history_data ? history_data->who : 0);
}
