//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004-2007 Peter Miller
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
// MANIFEST: functions to manipulate splits
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

    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
