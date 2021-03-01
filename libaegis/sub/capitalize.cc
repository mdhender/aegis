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

#include <common/mem.h>
#include <libaegis/sub.h>
#include <libaegis/sub/capitalize.h>
#include <common/trace.h>
#include <common/wstring/list.h>


//
// NAME
//	sub_capitalize - the capitalize substitution
//
// SYNOPSIS
//	string_ty *sub_capitalize(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_capitalize function implements the capitalize substitution.
//	The capitalize substitution is replaced by the single argument
//	mapped to upper case.
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
sub_capitalize(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_capitalize()\n{\n"));
    wstring result;
    if (arg.size() < 2)
    {
	scp->error_set(i18n("requires one argument"));
    }
    else
    {
	wstring wis = arg.unsplit(1, arg.size(), 0);
	result = wis.capitalize();
    }
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
