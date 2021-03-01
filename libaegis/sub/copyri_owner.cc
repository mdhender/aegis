//
//      aegis - project change supervisor
//      Copyright (C) 2006-2008, 2012 Peter Miller
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
#include <libaegis/change.h>
#include <libaegis/change/attributes.h>
#include <libaegis/sub/copyri_owner.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


wstring
sub_copyright_owner(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_copyright_owner()\n{\n"));
    wstring result;
    change::pointer cp = sub_context_change_get(scp);
    if (!cp)
    {
        scp->error_set(i18n("not valid in current context"));
        trace(("}\n"));
        return result;
    }
    if (arg.size() != 1)
    {
        scp->error_set(i18n("requires zero arguments"));
        trace(("}\n"));
        return result;
    }

    nstring value(cp->pconf_copyright_owner_get());
    result = wstring(value);
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
