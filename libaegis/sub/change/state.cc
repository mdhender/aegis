//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate states
//

#include <change.h>
#include <sub/change/state.h>
#include <sub.h>
#include <trace.h>
#include <wstr.h>
#include <wstr_list.h>


//
// NAME
//	sub_state - the state substitution
//
// SYNOPSIS
//	string_ty *sub_state(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_state function implements the state substitution.
//	The state substitution is replaced by the name of the state the
//	current change is in.
//
//	Requires exactly zero arguments.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_state(sub_context_ty *scp, wstring_list_ty *arg)
{
    change_ty	    *cp;
    wstring_ty	    *result;

    trace(("sub_state()\n{\n"));
    result = 0;
    cp = sub_context_change_get(scp);
    if (!cp)
	sub_context_error_set(scp, i18n("not valid in current context"));
    else
    {
	cstate_ty       *cstate_data;

	cstate_data = change_cstate_get(cp);
	result = wstr_from_c(cstate_state_ename(cstate_data->state));
    }
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
