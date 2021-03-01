//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate integra_names
//

#include <libaegis/change.h>
#include <common/error.h> // for assert
#include <common/trace.h>


string_ty *
change_integrator_name(change_ty *cp)
{
    cstate_ty       *cstate_data;
    cstate_history_ty *history_data;
    long            pos;

    trace(("change_integrator_name(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    cstate_data = change_cstate_get(cp);
    assert(cstate_data->history);
    history_data = 0;
    for (pos = cstate_data->history->length - 1; pos >= 0 ; --pos)
    {
	history_data = cstate_data->history->list[pos];
	switch (history_data->what)
	{
	case cstate_history_what_develop_begin:
	case cstate_history_what_develop_begin_undo:
	case cstate_history_what_develop_end:
	case cstate_history_what_develop_end_2ai:
	case cstate_history_what_develop_end_2ar:
	case cstate_history_what_develop_end_undo:
	case cstate_history_what_new_change:
	case cstate_history_what_review_begin:
	case cstate_history_what_review_begin_undo:
	case cstate_history_what_review_pass:
	case cstate_history_what_review_pass_2ar:
	case cstate_history_what_review_pass_2br:
	case cstate_history_what_review_pass_undo:
	case cstate_history_what_review_pass_undo_2ar:
	case cstate_history_what_review_fail:
#ifndef DEBUG
	default:
#endif
	    history_data = 0;
	    continue;

	case cstate_history_what_integrate_pass:
	case cstate_history_what_integrate_fail:
	case cstate_history_what_integrate_begin:
	case cstate_history_what_integrate_begin_undo:
	    break;
	}
	break;
    }
    trace(("return \"%s\";\n",
	history_data ? history_data->who->str_text : ""));
    trace(("}\n"));
    return (history_data ? history_data->who : 0);
}
