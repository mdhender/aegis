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

#include <common/trace.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/change/inventory.h>
#include <libaegis/change/functor/invento_list.h>
#include <libaegis/change/identifier.h>
#include <libaegis/project.h>
#include <libaegis/project/invento_walk.h>
#include <libaegis/user.h>


void
list_change_inventory(change_identifier &cid, string_list_ty *)
{
    trace(("list_change_inventory()\n{\n"));
    if (cid.set())
        list_change_inappropriate();

    //
    // create the columns
    //
    change_functor_inventory_list cf(false, cid.get_pp());

    //
    // Read the project change inventory.
    //
    project_inventory_walk(cid.get_pp(), cf);
    trace(("}\n"));
}
