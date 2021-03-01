//
//      aegis - project change supervisor
//      Copyright (C) 2002, 2004-2008, 2012 Peter Miller
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

#include <common/nstring/list.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/split.h>


wstring
sub_split(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_split()\n{\n"));
    if (arg.size() < 2)
    {
        scp->error_set(i18n("requires two arguments"));
        trace(("return NULL;\n"));
        trace(("}\n"));
        return wstring();
    }

    nstring separators = arg[1].to_nstring();
    nstring_list results;
    for (size_t j = 2; j < arg.size(); ++j)
    {
        nstring_list wl;
        wl.split(arg[j].to_nstring(), separators.c_str());
        results.push_back(wl);
    }
    wstring result(results.unsplit());

    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
