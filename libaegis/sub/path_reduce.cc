//
//      aegis - project change supervisor
//      Copyright (C) 2007, 2008, 2012 Peter Miller
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
#include <libaegis/sub/path_reduce.h>


wstring
sub_path_reduce(sub_context_ty *, const wstring_list &arg)
{
    trace(("sub_path_reduce()\n{\n"));
    nstring_list results;
    for (size_t j = 1; j < arg.size(); ++j)
    {
        nstring s = arg[j].to_nstring();
        if (s.empty())
            continue;
        nstring_list parts;
        parts.split(s, ":");
        results.push_back_unique(parts);
    }
    wstring result(results.unsplit(":"));
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
