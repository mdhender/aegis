//
//	aegis - project change supervisor
//	Copyright (C) 1996, 2003-2007 Peter Miller
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
// MANIFEST: functions to manipulate rights
//

#include <common/nstring.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/right.h>


wstring
sub_right(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_right()\n{\n"));
    if (arg.size() != 3)
    {
	scp->error_set(i18n("requires two arguments"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return wstring();
    }
    long n = arg[2].to_nstring().to_long();

    wstring result;
    if (n <= 0)
	;
    else if (n >= (long)arg[1].size())
	result = arg[1];
    else
    {
	wstring ws = arg[1];
	result = wstring(ws.c_str() + ws.size() - n, (size_t)n);
    }
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
