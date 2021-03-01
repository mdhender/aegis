//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2003-2008, 2012 Peter Miller
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

#include <common/libdir.h>
#include <libaegis/sub.h>
#include <libaegis/sub/common_direc.h>
#include <common/trace.h>
#include <common/wstring/list.h>


//
// NAME
//      sub_common_directory - the shared library substitution
//
// SYNOPSIS
//      string_ty *sub_common_directory(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_common_directory function implements the common_directory
//      substitution.  The common_directory substitution is replaced by
//      the absolute path of aegis' common (network shared writable)
//      data directory.  See ${data_directory} for the architecture
//      neutral (maybe readonly) library directory.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_common_directory(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_library()\n{\n"));
    wstring result;
    if (arg.size() != 1)
        scp->error_set(i18n("requires zero arguments"));
    else
        result = wstring(configured_comdir());
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
