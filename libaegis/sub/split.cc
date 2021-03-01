//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate splits
//

#include <str_list.h>
#include <sub.h>
#include <sub/split.h>
#include <trace.h>
#include <wstr_list.h>


wstring_ty *
sub_split(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty      *result;
    string_ty       *separators;
    string_ty       *s1;
    string_list_ty  results;
    size_t	    j;

    trace(("sub_split()\n{\n"));
    if (arg->nitems < 2)
    {
       	sub_context_error_set(scp, i18n("requires two arguments"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    separators = wstr_to_str(arg->item[1]);
    string_list_constructor(&results);
    for (j = 2; j < arg->nitems; ++j)
    {
	string_list_ty  wl;

	s1 = wstr_to_str(arg->item[j]);
	str2wl(&wl, s1, separators->str_text, 0);
	str_free(s1);
	string_list_append_list(&results, &wl);
	string_list_destructor(&wl);
    }
    str_free(separators);

    s1 = wl2str(&results, 0, results.nstrings, 0);
    result = str_to_wstr(s1);
    str_free(s1);

    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
