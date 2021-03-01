//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/change/attributes.h>
#include <common/error.h> // for assert


string_ty *
change_attributes_find(change::pointer cp, string_ty *name)
{
    assert(cp);
    cstate_ty *cstate_data = cp->cstate_get();
    assert(cstate_data);
    assert(name);
    attributes_ty *psp =
	attributes_list_find(cstate_data->attribute, name->str_text);
    if (psp)
    {
	assert(psp->value);
	return psp->value;
    }
    return 0;
}
