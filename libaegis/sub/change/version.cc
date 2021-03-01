//
//      aegis - project change supervisor
//      Copyright (C) 2002-2009, 2012 Peter Miller
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

#include <libaegis/change/branch.h>
#include <libaegis/sub/change/version.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <common/wstring/list.h>


//
// NAME
//      sub_version - the version substitution
//
// SYNOPSIS
//      string_ty *sub_version(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_version function implements the version substitution.
//      The version substitution is replaced by the version of the chnage.
//
//      If the change is not in the being_integrated or completed states,
//      the version will be major.minor.Cnumber, where
//      "major" is the project major version number,
//      "minor" is the project minor version number, and
//      "change" is a 3 digit change number, possibly zero padded.
//
//      If the change is in the being_integrated or completed state,
//      the version will be major.minor.Cdelta, where
//      "delta" is a 3 digit delta number, possibly zero padded.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_version(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_version()\n{\n"));
    bool uuid = false;
    for (size_t j = 1; j < arg.size(); ++j)
    {
        string_ty *s = wstr_to_str(arg[j].get_ref());
        static string_ty *uuid_name;
        if (!uuid_name)
            uuid_name = str_from_c("delta_uuid");
        if (str_equal(uuid_name, s))
            uuid = true;
        else
        {
            scp->error_set(i18n("requires zero arguments"));
            trace(("return NULL;\n"));
            trace(("}\n"));
            return wstring();
        }
    }

    change::pointer cp = sub_context_change_get(scp);
    if (!cp || cp->bogus)
    {
        scp->error_set(i18n("not valid in current context"));
        trace(("return NULL;\n"));
        trace(("}\n"));
        return wstring();
    }

    wstring result;
    if (uuid)
    {
        cstate_ty *cstate_data = cp->cstate_get();
        if (!cstate_data->delta_uuid)
        {
            scp->error_set(i18n("not valid in current context"));
            trace(("return NULL;\n"));
            trace(("}\n"));
            return wstring();
        }
        result = str_to_wstr(cstate_data->delta_uuid);
    }
    else
        result = wstring(cp->version_get());

    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
