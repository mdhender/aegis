//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2008 Peter Miller
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

#include <common/nstring/list.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/sub/dirname.h>


//
// NAME
//	sub_dirname - the dirname substitution
//
// SYNOPSIS
//	wstring_ty *sub_dirname(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_dirname function implements the dirname substitution.
//	The dirname substitution is replaced by the dirname of
//	the argument path, similar to the dirname(1) command.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring
sub_dirname(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_dirname()\n{\n"));
    wstring result;
    if (arg.size() < 2)
    {
       	scp->error_set(i18n("requires one argument"));
    }
    else
    {
	os_become_orig();
	nstring_list results;
	for (size_t j = 1; j < arg.size(); ++j)
	{
	    nstring s1 = arg[j].to_nstring();
	    nstring s2 = os_dirname(s1);
	    results.push_back(s2);
	}
	os_become_undo();
	result = wstring(results.unsplit());
    }
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
