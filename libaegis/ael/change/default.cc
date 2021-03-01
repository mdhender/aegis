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

#include <libaegis/ael/change/default.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/project.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>


void
list_default_change(string_ty *project_name, long change_number,
    string_list_ty *args)
{
    project_ty      *pp;
    user_ty::pointer up;

    //
    // check for silly arguments
    //
    trace(("list_default_change()\n{\n"));
    if (change_number)
	list_change_inappropriate();

    //
    // resolve the project name
    //
    if (!project_name)
	project_name = str_copy(user_ty::create()->default_project().get_ref());
    else
	project_name = str_copy(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    if (!args->nstrings)
	up = user_ty::create();
    else
	up = user_ty::create(nstring(args->string[0]));

    //
    // Find default change number;
    // will generate fatal error if no default.
    //
    change_number = up->default_change(pp);

    //
    // print it out
    //
    printf("%ld\n", magic_zero_decode(change_number));

    //
    // clean up and go home
    //
    project_free(pp);
    trace(("}\n"));
}
