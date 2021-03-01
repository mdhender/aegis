/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate architectures
 */

#include <change.h>
#include <project.h>
#include <sub/architecture.h>
#include <sub.h>
#include <trace.h>
#include <wstr.h>
#include <wstr_list.h>


/*
 * NAME
 *	sub_architecture - the architecture substitution
 *
 * SYNOPSIS
 *	string_ty *sub_architecture(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_architecture function implements the architecture
 *	substitution.  The architecture substitution is replaced by the
 *	architecture variant pattern appropriate for the current
 *	execution environment.	Requires no arguments.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_architecture(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty	    *result;

    trace(("sub_architecture()\n{\n"));
    if (arg->nitems > 1)
    {
	sub_context_error_set(scp, i18n("requires zero arguments"));
	result = 0;
    }
    else
    {
	change_ty	*cp;

	cp = sub_context_change_get(scp);
	if (!cp)
	{
	    project_ty	    *pp;

	    pp = sub_context_project_get(scp);
	    if (!pp)
	    {
		sub_context_error_set
		(
		    scp,
		    i18n("not valid in current context")
		);
		result = 0;
		goto done;
	    }
	    cp = project_change_get(pp);
	}
	result = str_to_wstr(change_architecture_name(cp, 1));
    }

    /*
     * here for all exits
     */
    done:
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
