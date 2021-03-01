//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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

#include <libaegis/change.h>
#include <libaegis/project.h>
#include <libaegis/sub/architecture.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <common/wstr.h>
#include <common/wstring/list.h>


//
// NAME
//	sub_architecture - the architecture substitution
//
// SYNOPSIS
//	string_ty *sub_architecture(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_architecture function implements the architecture
//	substitution.  The architecture substitution is replaced by the
//	architecture variant pattern appropriate for the current
//	execution environment.	Requires no arguments.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring
sub_architecture(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_architecture()\n{\n"));
    wstring result;
    if (arg.size() > 1)
    {
	scp->error_set(i18n("requires zero arguments"));
    }
    else
    {
	change::pointer cp = sub_context_change_get(scp);
	if (!cp)
	{
	    project_ty *pp = sub_context_project_get(scp);
	    if (!pp)
	    {
		scp->error_set(i18n("not valid in current context"));
		goto done;
	    }
	    cp = pp->change_get();
	}
	result = wstring(change_architecture_name(cp, 1));
    }

    //
    // here for all exits
    //
    done:
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
