//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2008 Peter Miller
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

#include <common/ac/stdio.h>

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/project/default.h>
#include <libaegis/ael/project/inappropriat.h>
#include <libaegis/change/identifier.h>
#include <libaegis/project.h>
#include <libaegis/user.h>


void
list_default_project(change_identifier &cid, string_list_ty *args)
{
    //
    // check for silly arguments
    //
    trace(("list_default_project()\n{\n"));
    if (cid.set())
	list_change_inappropriate();

    if (args && args->nstrings)
    {
	//
	// Use the user name supplied by the caller.
	//
	nstring login(args->string[0]);
        cid.set_user_by_name(login);
    }

    //
    // print it out
    //
    printf("%s\n", cid.get_pp()->name_get()->str_text);
    trace(("}\n"));
}
