//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate integra_lists
//

#include <libaegis/project/history.h>
#include <common/str_list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/project/integra_list.h>
#include <libaegis/sub/user.h>
#include <common/trace.h>
#include <libaegis/user.h>
#include <common/wstr.h>
#include <common/wstr/list.h>


//
// NAME
//	sub_integrator_list - the integrator_list substitution
//
// SYNOPSIS
//	wstring_ty *sub_integrator_list(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_integrator_list function implements the integrator_list
//	substitution.  The integrator_list substitution is replaced by a
//	space separated list of the project's integrators.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_integrator_list(sub_context_ty *scp, wstring_list_ty *arg)
{
    long j;
    string_ty *s;
    project_ty *pp;
    sub_user_func_ptr func;
    user_ty *up;

    trace(("sub_integrator_list()\n{\n"));
    pp = sub_context_project_get(scp);
    if (!pp)
    {
	sub_context_error_set(scp, i18n("not valid in current context"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }
    func = user_name;
    switch (arg->size())
    {
    default:
	sub_context_error_set(scp, i18n("requires one argument"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;

    case 1:
	break;

    case 2:
	s = wstr_to_str(arg->get(1));
	func = sub_user_func(s);
	str_free(s);
	if (!func)
	{
	    sub_context_error_set(scp, i18n("unknown substitution variant"));
	    trace(("return NULL;\n"));
	    trace(("}\n"));
	    return 0;
	}
	break;
    }

    //
    // build a string containing all of the project integrators
    //
    string_list_ty wl;
    for (j = 0; ; ++j)
    {
	s = project_integrator_nth(pp, j);
	if (!s)
	    break;
	up = user_symbolic(pp, s);
	s = func(up);
	wl.push_back(s);
    }
    s = wl.unsplit();
    wstring_ty *result = str_to_wstr(s);
    str_free(s);

    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
