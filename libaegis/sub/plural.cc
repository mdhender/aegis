//
//	aegis - project change supervisor
//	Copyright (C) 2002-2007 Peter Miller
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
// MANIFEST: functions to manipulate plurals
//

#include <common/trace.h>
#include <common/wstr.h>
#include <common/wstring/list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/plural.h>


wstring
sub_plural(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_plural()\n{\n"));
    wstring result;
    switch (arg.size())
    {
    default:
	scp->error_set(i18n("requires two or three arguments"));
	break;

    case 3:
        if (arg[1].to_nstring().to_long() != 1)
            result = arg[2];
	break;

    case 4:
        if (arg[1].to_nstring().to_long() != 1)
            result = arg[2];
        else
            result = arg[3];
	break;
    }
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
