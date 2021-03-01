/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001-2003 Peter Miller;
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
 * MANIFEST: functions to manipulate deltas
 */

#include <change.h>
#include <cstate.h>
#include <sub/change/delta.h>
#include <sub.h>
#include <wstr.h>
#include <wstr_list.h>
#include <trace.h>


/*
 * NAME
 *	sub_delta - the delta substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_delta(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_delta function implements the delta substitution.
 *	The delta substitution is replaced by the delta number of the project.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_delta(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty      *result;

    trace(("sub_delta()\n{\n"));
    result = 0;
    if (arg->nitems != 1)
    {
	sub_context_error_set(scp, i18n("requires zero arguments"));
    }
    else
    {
	change_ty	*cp;

	cp = sub_context_change_get(scp);
	if (!cp)
	{
	    yuck:
	    sub_context_error_set(scp, i18n("not valid in current context"));
	}
	else
	{
	    cstate_ty       *cstate_data;
	    string_ty       *s;

	    cstate_data = change_cstate_get(cp);
	    if (cstate_data->state < cstate_state_being_integrated)
		    goto yuck;
	    s = str_format("%ld", cstate_data->delta_number);
	    result = str_to_wstr(s);
	    str_free(s);
	}
    }
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
