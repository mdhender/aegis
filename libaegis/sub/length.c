/*
 *	aegis - project change supervisor
 *	Copyright (C) 1996, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate lengths
 */

#include <sub.h>
#include <sub/length.h>
#include <trace.h>
#include <wstr_list.h>


wstring_ty *
sub_length(scp, arg)
    sub_context_ty	*scp;
    wstring_list_ty	*arg;
{
    wstring_ty	*result;

    trace(("sub_length()\n{\n"/*}*/));
    if (arg->nitems < 2)
    {
       	sub_context_error_set(scp, i18n("requires one argument"));
	result = 0;
    }
    else
    {
	wstring_list_ty results;
	size_t		j;

	wstring_list_constructor(&results);
	for (j = 1; j < arg->nitems; ++j)
	{
    	    string_ty       *s;
    	    wstring_ty      *ws;

    	    s = str_format("%ld", (long)arg->item[j]->wstr_length);
    	    trace(("result = \"%s\";\n", s->str_text));
    	    ws = str_to_wstr(s);
    	    str_free(s);
	    wstring_list_append(&results, ws);
	    wstr_free(ws);
	}
	result = wstring_list_to_wstring(&results, 0, results.nitems, 0);
	wstring_list_destructor(&results);
    }
    trace(("return %8.8lX;\n", (long)result));
    trace((/*{*/"}\n"));
    return result;
}
