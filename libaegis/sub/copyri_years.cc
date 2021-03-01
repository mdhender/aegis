//
//      aegis - project change supervisor
//      Copyright (C) 2002-2008, 2012 Peter Miller
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

#include <common/trace.h>
#include <common/wstring/list.h>

#include <libaegis/change.h>
#include <libaegis/sub.h>
#include <libaegis/sub/copyri_years.h>


//
// NAME
//      sub_copyright_years - the change substitution
//
// SYNOPSIS
//      wstring_ty *sub_copyright_years(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_copyright_years function implements the change
//      substitution.  The copyright_years substitution is replaced by
//      the range of copyright years in the project's state, and
//      maintained at integrate begin time.  Do not use this to insert
//      into new files, it is not guaranteed to be up-to-date until the
//      integrate build, use ${date %Y} instead.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_copyright_years(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_copyright_years()\n{\n"));
    wstring result;
    if (arg.size() != 1)
    {
        scp->error_set(i18n("requires zero arguments"));
        trace(("}\n"));
        return result;
    }
    change::pointer cp = sub_context_change_get(scp);
    if (!cp || cp->bogus)
    {
        scp->error_set(i18n("not valid in current context"));
        trace(("}\n"));
        return result;
    }

    nstring s = cp->copyright_years_get_string();
    result = wstring(s);

    //
    // here for all exits
    //
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
