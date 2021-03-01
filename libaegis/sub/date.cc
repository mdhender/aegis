//
//      aegis - project change supervisor
//      Copyright (C) 2002-2008, 2012 Peter Miller
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

#include <common/ac/time.h>

#include <common/language.h>
#include <libaegis/sub.h>
#include <libaegis/sub/date.h>
#include <common/trace.h>
#include <common/wstr.h>
#include <common/wstring/list.h>


//
// NAME
//      sub_date - the date substitution
//
// SYNOPSIS
//      wstring_ty *sub_date(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_date function implements the date substitution.
//      The date substitution is replaced by the current date.
//      The optional arguments may construct format string,
//      similar to the date(1) command.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_date(sub_context_ty *, const wstring_list &arg)
{
    trace(("sub_date()\n{\n"));

    //
    // We use the time() function directly, rather than the now()
    // function.  This is because the now() function has a static time
    // (for timestamps, etc) by the progress indicators need the correct
    // time.
    //
    time_t when = 0;
    time(&when);

    wstring result;
    if (arg.size() < 2)
    {
        char *time_string = ctime(&when);
        result = wstring(time_string, 24);
    }
    else
    {
        nstring fmt = arg.unsplit(1, arg.size()).to_nstring();
        struct tm *theTm = localtime(&when);

        //
        // The strftime is locale dependent.
        //
        language_human();
        char buf[1000];
        strftime(buf, sizeof(buf), fmt.c_str(), theTm);
        language_C();
        result = wstring(buf);
    }
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
