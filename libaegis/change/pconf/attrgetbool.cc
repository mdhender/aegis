//
// aegis - project change supervisor
// Copyright (C) 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/boolean.h>
#include <common/nstring.h>
#include <libaegis/attribute.h>
#include <libaegis/change.h>


bool
change::pconf_attributes_get_boolean(const nstring &name, bool dflt)
{
    pconf_ty *pcp = change_pconf_get(this, 0);
    assert(pcp);
    if (!pcp)
        return dflt;
    attributes_ty *ap =
        attributes_list_find(pcp->project_specific, name.c_str());
    if (!ap)
        return dflt;
    assert(ap->value);
    if (!ap->value)
        return dflt;
    return string_to_bool(ap->value->str_text, dflt);
}


// vim: set ts=8 sw=4 et :
