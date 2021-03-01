//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2003-2008, 2012 Peter Miller
//      Copyright (C) 2008, 2009 Walter Franzini
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
#include <libaegis/sub/change/reviewer.h>
#include <libaegis/sub/user.h>
#include <libaegis/user.h>


//
// NAME
//      sub_reviewer - the reviewer substitution
//
// SYNOPSIS
//      wstring_ty *sub_reviewer(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_reviewer function implements the reviewer substitution.
//      The reviewer substitution is replaced by the name of the reviewer
//      of the change.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_reviewer(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_reviewer()\n{\n"));
    wstring result;
    change::pointer cp = sub_context_change_get(scp);
    if (!cp)
    {
        yuck:
        scp->error_set(i18n("not valid in current context"));
    }
    else if (arg.size() == 1)
    {
        nstring s(cp->reviewer_name());
        if (s.empty())
            goto yuck;
        result = wstring(s);
    }
    else if (arg.size() == 2)
    {
        nstring s = arg[1].to_nstring();
        sub_user_func_ptr func = sub_user_func(s);
        if (!func)
        {
            scp->error_set(i18n("unknown substitution variant"));
        }
        else
        {
            nstring who(cp->reviewer_name());
            user_ty::pointer up = user_ty::create(who);
            s = func(up);
            result = nstring(s);
        }
    }
    else
    {
        scp->error_set(i18n("requires one argument"));
    }
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
