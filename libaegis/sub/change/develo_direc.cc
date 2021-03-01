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
// MANIFEST: functions to manipulate develo_direcs
//

#include <change.h>
#include <sub/change/develo_direc.h>
#include <sub.h>
#include <trace.h>
#include <wstr.h>
#include <wstr/list.h>


//
// NAME
//	sub_development_directory - the development_directory substitution
//
// SYNOPSIS
//	wstring_ty *sub_development_directory(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_development_directory function implements the
//	development_directory substitution.  The development_directory
//	substitution is used to insert the absolute path of the
//	development_directory.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_development_directory(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty	    *result;
    cstate_ty       *cstate_data;

    trace(("sub_development_directory()\n{\n"));
    if (arg->size() != 1)
    {
	sub_context_error_set(scp, i18n("requires zero arguments"));
	result = 0;
    }
    else
    {
	change_ty *cp = sub_context_change_get(scp);
	if (!cp)
	{
	    yuck:
	    sub_context_error_set(scp, i18n("not valid in current context"));
	    result = 0;
	}
	else
	{
	    cstate_data = change_cstate_get(cp);
	    switch (cstate_data->state)
	    {
	    case cstate_state_awaiting_development:
	    case cstate_state_completed:
		goto yuck;

	    case cstate_state_being_developed:
	    case cstate_state_awaiting_review:
	    case cstate_state_being_reviewed:
	    case cstate_state_awaiting_integration:
	    case cstate_state_being_integrated:
		break;
	    }
	    result = str_to_wstr(change_development_directory_get(cp, 0));
	}
    }
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
