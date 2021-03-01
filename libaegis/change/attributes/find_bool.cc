//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller;
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
// MANIFEST: implementation of the change_attributes_find_bool class
//

#include <common/error.h> // for assert
#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/change/attributes.h>
#include <libaegis/cstate.h>


bool
change_attributes_find_boolean(change_ty *cp, const char *name, bool dflt)
{
    assert(cp);
    cstate_ty *cstate_data = change_cstate_get(cp);
    assert(cstate_data);
    assert(name);
    return attributes_list_find_boolean(cstate_data->attribute, name, dflt);
}
