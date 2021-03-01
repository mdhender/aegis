//
// aegis - project change supervisor
// Copyright (C) 1999, 2003-2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <common/str_list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/search_path.h>
#include <common/trace.h>
#include <common/wstring/list.h>


//
// NAME
//      sub_search_path - the search_path substitution
//
// SYNOPSIS
//      string_ty *sub_search_path(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_search_path function implements the search_path
//      substitution.  The search_path substitution is replaced by a
//      colon separated list of absolute paths to search when building a
//      change, it will point from a change to its branch and so on up
//      to the project trunk.
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
sub_search_path(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_search_path()\n{\n"));
    if (arg.size() != 1)
    {
        scp->error_set(i18n("requires zero arguments"));
        trace(("return NULL;\n"));
        trace(("}\n"));
        return wstring();
    }

    string_list_ty tmp;
    change::pointer cp = sub_context_change_get(scp);
    if (!cp)
    {
        project *pp = sub_context_project_get(scp);
        if (!pp)
        {
            scp->error_set(i18n("not valid in current context"));
            trace(("return NULL;\n"));
            trace(("}\n"));
            return wstring();
        }

        pp->search_path_get(&tmp, false);
    }
    else
        cp->search_path_get(&tmp, false);

    string_ty *s = tmp.unsplit(":");
    wstring result(s);
    str_free(s);

    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
