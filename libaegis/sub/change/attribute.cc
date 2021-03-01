//
//      aegis - project change supervisor
//      Copyright (C) 2004-2008, 2012 Peter Miller
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

#include <common/nstring.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/change/attributes.h>
#include <libaegis/sub.h>
#include <libaegis/sub/change/attribute.h>


wstring
sub_change_attribute(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_change_attribute()\n{\n"));
    change::pointer cp = scp->change_get();
    if (!cp)
    {
        scp->error_set(i18n("not valid in current context"));
        trace(("return NULL;\n"));
        trace(("}\n"));
        return wstring();
    }
    if (arg.size() != 2)
    {
        scp->error_set(i18n("requires one argument"));
        trace(("return NULL;\n"));
        trace(("}\n"));
        return wstring();
    }

    nstring name(arg[1].to_nstring());
    nstring value(change_attributes_find(cp, name.get_ref()));
    wstring result(value);
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
