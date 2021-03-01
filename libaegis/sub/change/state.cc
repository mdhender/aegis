//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/change.h>
#include <libaegis/sub/change/state.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <common/wstr.h>
#include <common/wstr/list.h>


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

wstring
sub_state(sub_context_ty *scp, const wstring_list &)
{
    trace(("sub_state()\n{\n"));
    wstring result;
    change::pointer cp = sub_context_change_get(scp);
    if (!cp)
	scp->error_set(i18n("not valid in current context"));
    else
    {
	cstate_ty *cstate_data = cp->cstate_get();
	result = wstring(cstate_state_ename(cstate_data->state));
    }
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
