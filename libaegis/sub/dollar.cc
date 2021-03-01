//
//      aegis - project change supervisor
//      Copyright (C) 1997, 2003-2008 Peter Miller
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
#include <libaegis/sub/dollar.h>
#include <common/trace.h>
#include <common/wstring/list.h>


//
// NAME
//      sub_dollar - substitute a dollar
//
// SYNOPSIS
//      string_ty *sub_dollar(wlist *arg);
//
// DESCRIPTION
//      The sub_dollar function implements the dollar substitution.
//      The dollar substitution is replaced by a dollar ($) character.
//
//      Requires exactly zero arguments.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_dollar(sub_context_ty *scp, const wstring_list &arg)
{
    wstring_ty      *result;

    trace(("sub_dollar()\n{\n"));
    if (arg.size() != 1)
    {
        scp->error_set(i18n("requires zero arguments"));
        result = 0;
    }
    else
        result = wstr_from_c("$");
    trace(("}\n"));
    return result;
}
