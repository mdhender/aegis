//
//	aegis - project change supervisor
//	Copyright (C) 1998, 2003-2008 Peter Miller
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

#include <common/nstring.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/trim_directo.h>


wstring
sub_trim_directory(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_trim_directory()\n{\n"));
    wstring dir;
    long n = 1;
    switch (arg.size())
    {
    default:
        scp->error_set(i18n("requires one argument"));
        trace(("return NULL;\n"));
        trace(("}\n"));
        return wstring();

    case 2:
        n = 1;
        dir = arg[1];
        break;

    case 3:
        {
            nstring s = arg[1].to_nstring();
            n = atol(s.c_str());
            trace(("n = %ld;\n", n));
            dir = arg[2];
        }
        break;
    }

    //
    // Skip the given number of leading directory components.
    //
    const wchar_t *wcp = dir.c_str();
    const wchar_t *wep = wcp + dir.size();
    while (n-- > 0)
    {
        const wchar_t *ep = wcp;
        while (ep < wep && *ep != '/')
            ++ep;
        if (ep >= wep)
            break;
        wcp = ep + 1;
    }

    //
    // Build the result.
    //
    wstring result(wcp, wep - wcp);
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
