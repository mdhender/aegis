/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to implement the add_path_suffix substitution
 */

#include <os.h>
#include <str_list.h>
#include <sub.h>
#include <sub/addpathsuffi.h>
#include <trace.h>
#include <wstr_list.h>


wstring_ty *
sub_add_path_suffix(sub_context_ty *scp, wstring_list_ty *arg)
{
    string_ty	    *s;
    wstring_ty	    *result;
    size_t	    j;
    string_list_ty  sl;
    string_ty	    *suffix;

    /*
     * Make sure we have sufficient arguments.
     */
    trace(("sub_add_path_suffix()\n{\n"));
    if (arg->nitems < 2)
    {
	sub_context_error_set(scp, i18n("requires at least one argument"));
	trace(("return NULL;\n"));
	trace(( /*{*/"}\n"));
	return 0;
    }

    /*
     * add the suffix to each of the paths in each of the arguments
     */
    suffix = wstr_to_str(arg->item[1]);
    string_list_constructor(&sl);
    for (j = 2; j < arg->nitems; ++j)
    {
	size_t		k;
	string_list_ty	tmp;

	s = wstr_to_str(arg->item[j]);
	str2wl(&tmp, s, ":", 0);
	str_free(s);
	for (k = 0; k < tmp.nstrings; ++k)
	{
	    s = os_path_cat(tmp.string[k], suffix);
	    string_list_append(&sl, s);
	    str_free(s);
	}
	string_list_destructor(&tmp);
    }
    str_free(suffix);
    s = wl2str(&sl, 0, sl.nstrings, ":");
    result = str_to_wstr(s);
    str_free(s);

    /*
     * success
     */
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
