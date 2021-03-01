//
//	aegis - project change supervisor
//	Copyright (C) 1996, 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate namemaxs
//

#include <os.h>
#include <str_list.h>
#include <sub.h>
#include <sub/namemax.h>
#include <trace.h>
#include <wstr/list.h>


wstring_ty *
sub_namemax(sub_context_ty *scp, wstring_list_ty *arg)
{
    trace(("sub_namemax()\n{\n"));
    if (arg->size() < 2)
    {
       	sub_context_error_set(scp, i18n("requires one argument"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    string_list_ty results;
    os_become_orig();
    for (size_t j = 1; j < arg->size(); ++j)
    {
	string_ty *s = wstr_to_str(arg->get(j));
	trace(("s = \"%s\";\n", s->str_text));
	int n = os_pathconf_name_max(s);
	str_free(s);
	s = str_format("%d", n);
	trace(("result = \"%s\";\n", s->str_text));
	results.push_back(s);
	str_free(s);
    }
    os_become_undo();

    string_ty *s = results.unsplit();
    wstring_ty *result = str_to_wstr(s);
    str_free(s);
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
