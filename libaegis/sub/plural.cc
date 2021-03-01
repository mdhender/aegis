//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate plurals
//

#include <libaegis/sub.h>
#include <libaegis/sub/plural.h>
#include <common/trace.h>
#include <common/wstr.h>
#include <common/wstr/list.h>


wstring_ty *
sub_plural(sub_context_ty *scp, wstring_list_ty *arg)
{
    string_ty	    *s;
    wstring_ty	    *result;
    long	    n;

    trace(("sub_plural()\n{\n"));
    switch (arg->size())
    {
    default:
	sub_context_error_set(scp, i18n("requires two or three arguments"));
	result = 0;
	break;

    case 3:
	arg->push_back(wstr_from_c(""));
	// fall through...

    case 4:
	s = wstr_to_str(arg->get(1));
	n = atol(s->str_text);
	str_free(s);
	if (n != 1)
	    result = wstr_copy(arg->get(2));
	else
	    result = wstr_copy(arg->get(3));
	break;
    }
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
