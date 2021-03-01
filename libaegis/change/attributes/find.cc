//
//      aegis - project change supervisor
//      Copyright (C) 2004-2008, 2012 Peter Miller
//      Copyright (C) 2008 Walter Franzini
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

#include <common/ac/assert.h>

#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/change/attributes.h>


string_ty *
change_attributes_find(change::pointer cp, const char *name)
{
    assert(cp);
    cstate_ty *cstate_data = cp->cstate_get();
    assert(cstate_data);
    assert(name && *name);
    attributes_ty *psp =
        attributes_list_find(cstate_data->attribute, name);
    if (psp)
    {
        assert(psp->value);
        return psp->value;
    }
    return 0;

}


string_ty *
change_attributes_find(change::pointer cp, string_ty *name)
{
    assert(name);
    return change_attributes_find(cp, name->str_text);
}


// vim: set ts=8 sw=4 et :
