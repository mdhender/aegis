//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2001-2005 Peter Miller;
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
// MANIFEST: functions to impliment the basename substitution
//

#include <common/ac/string.h>

#include <common/str.h>
#include <libaegis/sub.h>
#include <libaegis/sub/basename.h>
#include <libaegis/os.h>
#include <common/trace.h>
#include <common/wstr/list.h>


//
// NAME
//	sub_basename - the basename substitution
//
// SYNOPSIS
//	wstring_ty *sub_basename(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_basename function implements the basename substitution.
//	The basename substitution is replaced by the basename of
//	the argument path, similar to the basename(1) command.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_basename(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty	    *result;
    string_ty	    *suffix;
    string_ty	    *s1;
    string_ty	    *s2;

    trace(("sub_basename()\n{\n"));
    switch (arg->size())
    {
    default:
	sub_context_error_set(scp, i18n("requires one or two arguments"));
	result = 0;
	break;

    case 2:
	s1 = wstr_to_str(arg->get(1));
	s2 = os_basename(s1);
	str_free(s1);
	result = str_to_wstr(s2);
	str_free(s2);
	break;

    case 3:
	s1 = wstr_to_str(arg->get(1));
        suffix = wstr_to_str(arg->get(2));
        s2 = os_basename(s1, suffix);
        result = str_to_wstr(s2);
        str_free(s1);
        str_free(s2);
	str_free(suffix);
	break;
    }
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
