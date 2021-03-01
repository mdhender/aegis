//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2002-2008, 2012 Peter Miller
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

#include <common/nstring.h>
#include <common/regula_expre.h>
#include <libaegis/sub.h>
#include <libaegis/sub/substitute.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <common/wstring.h>


wstring
sub_substitute(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_substitute()\n{\n"));

    //
    // make sure there are enough arguments
    //
    if (arg.size() < 3)
    {
        scp->error_set(i18n("requires two or more arguments"));
        trace(("return NULL;\n"));
        trace(("}\n"));
        return wstring();
    }
    nstring lhs(arg[1].to_nstring());
    nstring rhs(arg[2].to_nstring());

    //
    // turn it into one big string to be substituted within
    //
    nstring s = arg.unsplit(3, arg.size()).to_nstring();

    //
    // do the substitution
    //
    regular_expression re(lhs);
    nstring narrow_result;
    if (!re.match_and_substitute(rhs, s, 0, narrow_result))
    {
        scp->error_set(re.strerror());
        trace(("return NULL;\n"));
        trace(("}\n"));
        return wstring();
    }

    //
    // clean up and return
    //
    wstring result(narrow_result);
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
