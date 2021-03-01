//
//	aegis - project change supervisor
//	Copyright (C) 1996, 2002-2008 Peter Miller
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
#include <libaegis/sub/namemax.h>


wstring
sub_namemax(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_namemax()\n{\n"));
    wstring result;
    if (arg.size() < 2)
    {
       	scp->error_set(i18n("requires one argument"));
	trace(("}\n"));
	return result;
    }

    nstring_list results;
    os_become_orig();
    for (size_t j = 1; j < arg.size(); ++j)
    {
	nstring s = arg[j].to_nstring();
	trace(("s = %s;\n", s.quote_c().c_str()));
	int n = os_pathconf_name_max(s);
	s = nstring::format("%d", n);
	trace(("result = %s;\n", s.quote_c().c_str()));
	results.push_back(s);
    }
    os_become_undo();

    nstring s = results.unsplit();
    result = wstring(s);
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
