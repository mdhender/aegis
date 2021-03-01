//
//      aegis - project change supervisor
//      Copyright (C) 1996, 2002-2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <libaegis/sub.h>
#include <libaegis/sub/expr.h>
#include <libaegis/sub/expr_gram.h>
#include <common/str.h>
#include <common/trace.h>
#include <common/wstring/list.h>


wstring
sub_expression(sub_context_ty *scp, const wstring_list &arg)
{
    //
    // Check the number of arguments
    //
    trace(("sub_expression()\n{\n" ));
    wstring result;
    if (arg.size() < 2)
    {
        scp->error_set(i18n("requires at least one argument"));
        trace(("}\n"));
        return result;
    }

    //
    // Fold all of the arguments into a single string,
    // and parse that string for an expression.
    //
    nstring s1 = arg.unsplit(1, arg.size()).to_nstring();
    trace(("s1 = %s;\n", s1.quote_c().c_str()));
    nstring s2 = sub_expr_gram(scp, s1);
    if (s2.empty())
    {
        trace(("}\n"));
        return result;
    }

    //
    // Turns the result of the parse into a wide string.
    //
    trace(("result = %s;\n", s2.quote_c().c_str()));
    result = wstring(s2);
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
