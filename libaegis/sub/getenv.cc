//
//      aegis - project change supervisor
//      Copyright (C) 2001-2008, 2012 Peter Miller
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

#include <common/error.h>
#include <common/nstring/list.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/getenv.h>



//
// NAME
//      sub_getenv - the getenv substitution
//
// SYNOPSIS
//      string_ty *sub_getenv(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_getenv function implements the getenv substitution.
//      The getenv substitution is replaced by the corresponding
//      environment variable (or empty if undefined).
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_getenv(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_getenv()\n{\n"));
    wstring result;
    if (arg.size() < 2)
    {
        scp->error_set(i18n("requires one argument"));
    }
    else
    {
        nstring_list results;
        for (size_t j = 1; j < arg.size(); ++j)
        {
            nstring name = arg[j].to_nstring();
            const char *value = getenv(name.c_str());
            if (!value)
                value = "";
            nstring s(value);
            results.push_back(s);
        }
        result = wstring(results.unsplit());
    }
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
