//
// aegis - project change supervisor
// Copyright (C) 2006-2008, 2012 Peter Miller
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

#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/change/attributes.h>
#include <libaegis/cstate.fmtgen.h>


bool
change_attributes_find_boolean(change::pointer cp, const char *name, bool dflt)
{
    assert(cp);
    cstate_ty *cstate_data = cp->cstate_get();
    assert(cstate_data);
    assert(name);
    return attributes_list_find_boolean(cstate_data->attribute, name, dflt);
}


// vim: set ts=8 sw=4 et :
