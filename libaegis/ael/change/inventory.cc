//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller;
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
// MANIFEST: implementation of the ael_change_inventory class
//

#include <libaegis/ael/change/inventory.h>
#include <libaegis/change/functor/invento_list.h>
#include <libaegis/project.h>
#include <libaegis/project/invento_walk.h>
#include <common/trace.h>
#include <libaegis/user.h>


void
list_change_inventory(string_ty *project_name, long change_number,
    string_list_ty *args)
{
    trace(("list_change_inventory()\n{\n"));
    if (!project_name)
    {
	user_ty *up = user_executing((project_ty *)0);
	project_name = user_default_project_by_user(up);
    }
    else
	project_name = str_copy(project_name);
    project_ty *pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // create the columns
    //
    change_functor_inventory_list cf(false, pp);

    //
    // Read the project change inventory.
    //
    project_inventory_walk(pp, cf);

    //
    // clean up and go home
    //
    project_free(pp);
    trace(("}\n"));
}
