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

#include <libaegis/sub.h>
#include <libaegis/sub/identifier.h>
#include <common/trace.h>
#include <common/wstr.h>
#include <common/wstring/list.h>


//
// NAME
//	sub_identifier - the identifier substitution
//
// SYNOPSIS
//	string_ty *sub_identifier(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_identifier function implements the identifier substitution.
//	The identifier substitution is replaced by the single argument
//	mapped to a C identifier.  All characters which are not legal in
//	a C identifier are replaced by an underscore.
//
//	Requires exactly one argument.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring
sub_identifier(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_identifier()\n{\n"));
    wstring result;
    if (arg.size() < 2)
    {
	scp->error_set(i18n("requires one argument"));
    }
    else
    {
	wstring_list results;
	for (size_t j = 1; j < arg.size(); ++j)
	{
	    results.push_back(arg[j].identifier());
	}
	result = results.unsplit();
    }
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
