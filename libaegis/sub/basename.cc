//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2001-2008 Peter Miller
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

#include <common/ac/string.h>

#include <common/str.h>
#include <libaegis/sub.h>
#include <libaegis/sub/basename.h>
#include <libaegis/os.h>
#include <common/trace.h>
#include <common/wstring/list.h>


//
// NAME
//	sub_basename - the basename substitution
//
// SYNOPSIS
//	wstring_ty *sub_basename(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_basename function implements the basename substitution.
//	The basename substitution is replaced by the basename of
//	the argument path, similar to the basename(1) command.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring
sub_basename(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_basename()\n{\n"));
    wstring result;
    switch (arg.size())
    {
    default:
	scp->error_set(i18n("requires one or two arguments"));
	break;

    case 2:
        {
            nstring s1 = arg[1].to_nstring();
            nstring s2 = os_basename(s1);
            result = wstring(s2);
        }
	break;

    case 3:
        {
            nstring s1 = arg[1].to_nstring();
            nstring suffix = arg[2].to_nstring();
            nstring s2 = os_basename(s1, suffix);
            result = wstring(s2);
        }
	break;
    }
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
