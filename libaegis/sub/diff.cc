//
//      aegis - project change supervisor
//      Copyright (C) 2005-2008, 2012 Peter Miller
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

#include <libaegis/sub.h>
#include <libaegis/sub/diff.h>
#include <common/trace.h>
#include <common/wstring/list.h>


wstring
sub_diff(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_diff()\n{\n"));
    wstring result;
    if (arg.size() != 1)
        scp->error_set(i18n("requires zero arguments"));
    else
        result = wstring(CONF_DIFF);
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
