//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate dirnames
//

#include <os.h>
#include <str.h>
#include <str_list.h>
#include <sub.h>
#include <sub/dirname.h>
#include <trace.h>
#include <wstr_list.h>


//
// NAME
//	sub_dirname - the dirname substitution
//
// SYNOPSIS
//	wstring_ty *sub_dirname(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_dirname function implements the dirname substitution.
//	The dirname substitution is replaced by the dirname of
//	the argument path, similar to the dirname(1) command.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_dirname(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty	*result;

    trace(("sub_dirname()\n{\n"));
    if (arg->nitems < 2)
    {
       	sub_context_error_set(scp, i18n("requires one argument"));
       	result = 0;
    }
    else
    {
	string_list_ty	results;
	size_t		j;
       	string_ty	*s1;

	os_become_orig();
	string_list_constructor(&results);
	for (j = 1; j < arg->nitems; ++j)
	{
	    string_ty	    *s2;

	    s1 = wstr_to_str(arg->item[j]);
	    s2 = os_dirname(s1);
	    str_free(s1);
	    string_list_append(&results, s2);
	    str_free(s2);
	}
	os_become_undo();
	s1 = wl2str(&results, 0, results.nstrings, 0);
	result = str_to_wstr(s1);
	str_free(s1);
    }
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
