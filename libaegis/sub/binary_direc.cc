//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2003-2007 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate binary_direcs
//

#include <common/libdir.h>
#include <libaegis/sub.h>
#include <libaegis/sub/binary_direc.h>
#include <common/wstring/list.h>
#include <common/trace.h>


//
// NAME
//      sub_binary_directory - the shared library substitution
//
// SYNOPSIS
//      string_ty *sub_binary_directory(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_binary_directory function implements the binary_directory
//      substitution.  The binary_directory substitution is replaced by the
//      absolute path of aegis' (architecture sprecific) binary directory.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_binary_directory(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_library()\n{\n"));
    wstring result;
    if (arg.size() != 1)
    {
        scp->error_set(i18n("requires zero arguments"));
    }
    else
        result = wstring(configured_bindir());
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
