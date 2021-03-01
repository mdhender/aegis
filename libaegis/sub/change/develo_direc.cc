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
#include <libaegis/sub/change/develo_direc.h>


//
// NAME
//      sub_development_directory - the development_directory substitution
//
// SYNOPSIS
//      wstring_ty *sub_development_directory(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_development_directory function implements the
//      development_directory substitution.  The development_directory
//      substitution is used to insert the absolute path of the
//      development_directory.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_development_directory(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_development_directory()\n{\n"));
    wstring result;
    if (arg.size() != 1)
    {
        scp->error_set(i18n("requires zero arguments"));
    }
    else
    {
        change::pointer cp = sub_context_change_get(scp);
        if (!cp)
        {
            yuck:
            scp->error_set(i18n("not valid in current context"));
        }
        else
        {
            cstate_ty *cstate_data = cp->cstate_get();
            switch (cstate_data->state)
            {
            case cstate_state_awaiting_development:
            case cstate_state_completed:
                goto yuck;

            case cstate_state_being_developed:
            case cstate_state_awaiting_review:
            case cstate_state_being_reviewed:
            case cstate_state_awaiting_integration:
            case cstate_state_being_integrated:
                break;
            }
            result = wstring(change_development_directory_get(cp, 0));
        }
    }
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
