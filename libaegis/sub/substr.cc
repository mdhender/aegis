//
//      aegis - project change supervisor
//      Copyright (C) 2003-2007 Peter Miller
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
// MANIFEST: functions to manipulate substrs
//

#include <common/nstring.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/substr.h>


wstring
sub_substr(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_substr()\n{\n"));
    if (arg.size() != 4)
    {
        scp->error_set(i18n("requires three arguments"));
        trace(("return NULL;\n"));
        trace(("}\n"));
        return wstring();
    }

    long n1 = arg[2].to_nstring().to_long();
    long n2 = arg[3].to_nstring().to_long();
    if (n1 < 0)
    {
        n2 += n1;
        n1 = 0;
    }

    wstring ws = arg[1];
    wstring result;
    if (n2 <= 0 || n1 >= (long)ws.size())
        ;
    else if (n1 + n2 > (long)ws.size())
        result = wstring(ws.c_str() + n1, ws.size() - n1);
    else
        result = wstr_n_from_wc(ws.c_str() + n1, (size_t)n2);
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
