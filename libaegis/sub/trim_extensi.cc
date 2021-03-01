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

#include <common/mem.h>
#include <libaegis/sub.h>
#include <libaegis/sub/trim_extensi.h>
#include <common/trace.h>
#include <common/wstring/list.h>


wstring
sub_trim_extension(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_trim_extension()\n{\n"));
    if (arg.size() != 2)
    {
	scp->error_set(i18n("requires one argument"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return wstring();
    }

    wstring fn = arg[1];
    wstring result(fn);
    const wchar_t *wcp = fn.c_str() + fn.size();
    while (wcp > fn.c_str() && wcp[-1] != '/')
    {
	--wcp;
	if (*wcp == '.')
	{
	    if (wcp <= fn.c_str() || wcp[-1] == '/')
		break;
	    result = wstring(fn.c_str(), wcp - fn.c_str());
            break;
	}
    }
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
