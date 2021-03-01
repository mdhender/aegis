//
//      aegis - project change supervisor
//      Copyright (C) 2003-2008, 2012 Peter Miller
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

    wstring s = arg[1];
    wstring result;
    if (n2 <= 0 || n1 >= (long)s.size())
        ;
    else if (n1 + n2 > (long)s.size())
        result = wstring(s.c_str() + n1, s.size() - n1);
    else
        result = wstr_n_from_wc(s.c_str() + n1, (size_t)n2);
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
