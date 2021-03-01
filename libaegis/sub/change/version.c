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
 * MANIFEST: functions to manipulate versions
 */

#include <change/branch.h>
#include <sub/change/version.h>
#include <sub.h>
#include <trace.h>
#include <wstr.h>
#include <wstr_list.h>


/*
 * NAME
 *	sub_version - the version substitution
 *
 * SYNOPSIS
 *	string_ty *sub_version(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_version function implements the version substitution.
 *	The version substitution is replaced by the version of the chnage.
 *
 *	If the change is not in the being_integrated or completed states,
 *	the version will be major.minor.Cnumber, where
 *	"major" is the project major version number,
 *	"minor" is the project minor version number, and
 *	"change" is a 3 digit change number, possibly zero padded.
 *
 *	If the change is in the being_integrated or completed state,
 *	the version will be major.minor.Cdelta, where
 *	"delta" is a 3 digit delta number, possibly zero padded.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_version(scp, arg)
    sub_context_ty  *scp;
    wstring_list_ty *arg;
{
    wstring_ty	    *result;

    trace(("sub_version()\n{\n"));
    result = 0;
    if (arg->nitems != 1)
	sub_context_error_set(scp, i18n("requires zero arguments"));
    else
    {
	change_ty	*cp;

	cp = sub_context_change_get(scp);
	if (!cp || cp->bogus)
	{
	    sub_context_error_set(scp, i18n("not valid in current context"));
	    result = 0;
	}
	else
	{
	    string_ty	    *s2;

	    s2 = change_version_get(cp);
	    result = str_to_wstr(s2);
	    str_free(s2);
	}
    }
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
