//
//	aegis - project change supervisor
//	Copyright (C) 2004 Walter Franzini
//	Copyright (C) 2008 Peter Miller
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

#include <common/error.h>              // for assert
#include <libaegis/attribute.h>
#include <libaegis/change/attributes.h>
#include <libaegis/cstate.h>


void
change_attributes_append(cstate_ty *cstate_data, const char *name,
    const char *value)
{
    assert(cstate_data);
    assert(name);
    assert(value);

    if (!cstate_data->attribute)
    {
        cstate_data->attribute =
	    (attributes_list_ty *)attributes_list_type.alloc();
    }
    attributes_list_append(cstate_data->attribute, name, value);
}
