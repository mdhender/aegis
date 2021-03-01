//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2008 Peter Miller
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

#include <common/nstring/list.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/sub/addpathsuffi.h>


wstring
sub_add_path_suffix(sub_context_ty *scp, const wstring_list &arg)
{
    //
    // Make sure we have sufficient arguments.
    //
    trace(("sub_add_path_suffix()\n{\n"));
    if (arg.size() < 2)
    {
	scp->error_set(i18n("requires at least one argument"));
	trace(("}\n"));
	return wstring();
    }

    //
    // add the suffix to each of the paths in each of the arguments
    //
    nstring suffix = arg[1].to_nstring();
    nstring_list sl;
    for (size_t j = 2; j < arg.size(); ++j)
    {
	nstring s = arg[j].to_nstring();
	nstring_list tmp;
	tmp.split(s, ":");
	for (size_t k = 0; k < tmp.size(); ++k)
	{
	    s = os_path_cat(tmp[k], suffix);
	    sl.push_back(s);
	}
    }
    wstring result(sl.unsplit(":"));

    //
    // success
    //
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
