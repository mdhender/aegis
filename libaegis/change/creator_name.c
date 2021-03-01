/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate creator_names
 */

#include <change.h>
#include <error.h> /* for assert */
#include <trace.h>


string_ty *
change_creator_name(change_ty *cp)
{
    cstate	    cstate_data;
    string_ty       *who;

    trace(("change_creator_name(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    cstate_data = change_cstate_get(cp);
    assert(cstate_data->history);
    who = cstate_data->history->list[0]->who;
    trace(("return \"%s\";\n", who ? who->str_text : ""));
    trace(("}\n"));
    return who;
}
