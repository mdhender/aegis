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

#include <common/mem.h>
#include <libaegis/sub.h>
#include <libaegis/sub/upcase.h>
#include <common/trace.h>
#include <common/wstring/list.h>


//
// NAME
//      sub_upcase - the upcase substitution
//
// SYNOPSIS
//      string_ty *sub_upcase(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_upcase function implements the upcase substitution.
//      The upcase substitution is replaced by the single argument
//      mapped to upper case.
//
//      Requires exactly one argument.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_upcase(sub_context_ty *scp, const wstring_list &arg)
{
    wstring result;
    trace(("sub_upcase()\n{\n"));
    if (arg.size() < 2)
    {
        scp->error_set(i18n("requires one argument"));
    }
    else
    {
        wstring wis = arg.unsplit(1, arg.size());
        result = wis.upcase();
    }
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
