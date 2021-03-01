//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2003-2008 Peter Miller
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
#include <libaegis/sub/data_directo.h>
#include <common/trace.h>
#include <common/wstring/list.h>


//
// NAME
//      sub_data_directory - the shared library substitution
//
// SYNOPSIS
//      string_ty *sub_data_directory(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_data_directory function implements the data_directory
//      substitution.  The data_directory substitution is replaced by the
//      absolute path of aegis' (architecture neutral) library directory.
//      See ${library_directory} for the architecture specific library
//      directory.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_data_directory(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_library()\n{\n"));
    wstring result;
    if (arg.size() != 1)
        scp->error_set(i18n("requires zero arguments"));
    else
        result = wstring(configured_datadir());
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
