//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2007 Peter Miller
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
// MANIFEST: functions to manipulate defaults
//

#include <common/ac/stdio.h>

#include <libaegis/ael/project/default.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/project/inappropriat.h>
#include <libaegis/project.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/user.h>


void
list_default_project(string_ty *project_name, long change_number,
    string_list_ty *args)
{
    //
    // check for silly arguments
    //
    trace(("list_default_project()\n{\n"));
    if (change_number)
	list_change_inappropriate();

    //
    // Find default project name;
    // will generate fatal error if no default.
    //
    if (!args->nstrings)
    {
        user_ty::pointer up = user_ty::create();
        nstring n = up->default_project();
	project_name = str_copy(n.get_ref());
    }
    else
    {
	//
	// Use the user name supplied by the caller.
	//
	nstring login(args->string[0]);
	user_ty::pointer up = user_ty::create(login);
        nstring n = up->default_project();
	project_name = str_copy(n.get_ref());
    }

    //
    // Bind to the project.  This detects non-existent pojects, and
    // allows us to turn project aliases into exact project names.
    //
    project_ty *pp = project_alloc(project_name);
    pp->bind_existing();

    //
    // print it out
    //
    printf("%s\n", project_name_get(pp)->str_text);
    project_free(pp);
    trace(("}\n"));
}
