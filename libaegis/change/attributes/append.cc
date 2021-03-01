//
//	aegis - project change supervisor
//	Copyright (C) 2004 Walter Franzini;
//	All rights reserved.
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
// MANIFEST: implementation of the change_attributes_append class
//

#include <attribute.h>
#include <cstate.h>
#include <change/attributes.h>
#include <error.h>              // for assert


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
