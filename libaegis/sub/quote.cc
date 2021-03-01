//
//	aegis - project change supervisor
//	Copyright (C) 1998, 2003-2007 Peter Miller
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
// MANIFEST: functions to implement the quote substitution
//

#include <libaegis/sub.h>
#include <libaegis/sub/quote.h>
#include <common/trace.h>
#include <common/wstring/list.h>


wstring
sub_quote(sub_context_ty *, const wstring_list &arg)
{
    //
    // Convert the work list to a single string.
    //
    trace(("sub_quote()\n{\n"));
    nstring s = arg.unsplit(1, arg.size()).to_nstring();

    //
    // quote the string as required
    //
    wstring result(s.quote_shell());

    //
    // clean up and return
    //
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
