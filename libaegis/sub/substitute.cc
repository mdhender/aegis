//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate substitutes
//

#include <nstring.h>
#include <regula_expre.h>
#include <sub.h>
#include <sub/substitute.h>
#include <trace.h>
#include <wstr/list.h>
#include <wstring.h>


wstring_ty *
sub_substitute(sub_context_ty *scp, wstring_list_ty *arg)
{
    //
    // make sure there are enough arguments
    //
    trace(("sub_substitute()\n{\n"));
    if (arg->size() < 3)
    {
	sub_context_error_set(scp, i18n("requires two or more arguments"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }
    nstring lhs(wstr_to_str(arg->get(1)));
    nstring rhs(wstr_to_str(arg->get(2)));

    //
    // turn it into one big string to be substituted within
    //
    wstring ws = arg->unsplit(3, arg->size());
    nstring s(wstr_to_str(ws.get_ref()));

    //
    // do the substitution
    //
    regular_expression re(lhs);
    nstring output;
    if (!re.match_and_substitute(rhs, s, 0, output))
    {
	sub_context_error_set(scp, re.strerror());
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    //
    // clean up and return
    //
    wstring_ty *result = str_to_wstr(output.get_ref());
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
