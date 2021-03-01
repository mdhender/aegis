//
//	aegis - project change supervisor
//	Copyright (C) 2007 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
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
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
