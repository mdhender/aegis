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
// MANIFEST: functions to manipulate versions
//

#include <change/branch.h>
#include <sub/change/version.h>
#include <sub.h>
#include <trace.h>
#include <wstr.h>
#include <wstr/list.h>


//
// NAME
//	sub_version - the version substitution
//
// SYNOPSIS
//	string_ty *sub_version(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_version function implements the version substitution.
//	The version substitution is replaced by the version of the chnage.
//
//	If the change is not in the being_integrated or completed states,
//	the version will be major.minor.Cnumber, where
//	"major" is the project major version number,
//	"minor" is the project minor version number, and
//	"change" is a 3 digit change number, possibly zero padded.
//
//	If the change is in the being_integrated or completed state,
//	the version will be major.minor.Cdelta, where
//	"delta" is a 3 digit delta number, possibly zero padded.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_version(sub_context_ty *scp, wstring_list_ty *arg)
{
    trace(("sub_version()\n{\n"));
    bool uuid = false;
    for (size_t j = 1; j < arg->size(); ++j)
    {
	string_ty *s = wstr_to_str(arg->get(j));
	static string_ty *uuid_name;
	if (!uuid_name)
	    uuid_name = str_from_c("delta_uuid");
	if (str_equal(uuid_name, s))
	    uuid = true;
	else
	{
	    sub_context_error_set(scp, i18n("requires zero arguments"));
    	    trace(("return NULL;\n"));
    	    trace(("}\n"));
    	    return 0;
	}
    }

    change_ty *cp = sub_context_change_get(scp);
    if (!cp || cp->bogus)
    {
	sub_context_error_set(scp, i18n("not valid in current context"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    wstring_ty *result = 0;
    if (uuid)
    {
	cstate_ty *cstate_data = change_cstate_get(cp);
	if (!cstate_data->delta_uuid)
	{
	    sub_context_error_set(scp, i18n("not valid in current context"));
	    trace(("return NULL;\n"));
	    trace(("}\n"));
	    return 0;
	}
	result = str_to_wstr(cstate_data->delta_uuid);
    }
    else
    {
	string_ty *s2 = change_version_get(cp);
	result = str_to_wstr(s2);
	str_free(s2);
    }

    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
