//
//	aegis - project change supervisor
//	Copyright (C) 2001-2005 Peter Miller;
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
// MANIFEST: functions to manipulate getenvs
//

#include <common/error.h>
#include <common/str_list.h>
#include <libaegis/sub/getenv.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <common/wstr.h>
#include <common/wstr/list.h>



//
// NAME
//	sub_getenv - the getenv substitution
//
// SYNOPSIS
//	string_ty *sub_getenv(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_getenv function implements the getenv substitution.
//	The getenv substitution is replaced by the corresponding
//	environment variable (or empty if undefined).
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_getenv(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty	    *result;

    trace(("sub_getenv()\n{\n"));
    if (arg->size() < 2)
    {
       	sub_context_error_set(scp, i18n("requires one argument"));
       	result = 0;
    }
    else
    {
	string_list_ty results;
	for (size_t j = 1; j < arg->size(); ++j)
	{
	    string_ty *name = wstr_to_str(arg->get(j));
	    const char *value = getenv(name->str_text);
	    str_free(name);
	    if (!value)
		value = "";
	    string_ty *s = str_from_c(value);
	    results.push_back(s);
	    str_free(s);
	}
	string_ty *s = results.unsplit();
	result = str_to_wstr(s);
	str_free(s);
    }
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
