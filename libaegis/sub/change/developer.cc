//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/nstring.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/change.h>
#include <libaegis/sub.h>
#include <libaegis/sub/change/developer.h>
#include <libaegis/sub/user.h>
#include <libaegis/user.h>


//
// NAME
//	sub_developer - the developer substitution
//
// SYNOPSIS
//	wstring_ty *sub_developer(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_developer function implements the developer substitution.
//	The developer substitution is replaced by the name of the developer
//	of the change.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring
sub_developer(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_developer()\n{\n"));
    change::pointer cp = sub_context_change_get(scp);
    wstring result;
    if (!cp)
    {
        yuck:
        scp->error_set(i18n("not valid in current context"));
    }
    else if (arg.size() == 1)
    {
        result = wstring(change_developer_name(cp));
        if (result.empty())
            goto yuck;
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
            nstring login_name(change_developer_name(cp));
            user_ty::pointer up = user_ty::create(login_name);
            s = func(up);
            result = wstring(s);
        }
    }
    else
    {
        scp->error_set(i18n("requires one argument"));
    }
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
