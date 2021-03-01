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
// MANIFEST: implementation of the ael_project_details class
//

#include <libaegis/ael/change/details.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/project/details.h>
#include <libaegis/project.h>
#include <common/trace.h>
#include <libaegis/user.h>


void
list_project_details(string_ty *project_name, long change_number,
    string_list_ty *args)
{
    //
    // locate project data
    //
    trace(("list_project_details()\n{\n"));
    if (change_number)
	list_change_inappropriate();
    if (!project_name)
	project_name = user_default_project();
    else
	project_name = str_copy(project_name);
    project_ty *pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // List the project details.
    //
    list_change_details_columns process;
    process.list(pp->change_get(), true);

    //
    // Done.
    //
    project_free(pp);
    trace(("}\n"));
}
