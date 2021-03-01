//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2007 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//
// MANIFEST: implementation of the ael_branch_files class
//

#include <common/trace.h>
#include <libaegis/ael/branch/files.h>
#include <libaegis/ael/change/files.h>
#include <libaegis/project.h>
#include <libaegis/user.h>


void
list_branch_files(string_ty *project_name, long, string_list_ty *args)
{
    trace(("list_branch_files()\n{\n"));
    if (!project_name)
    {
        user_ty::pointer up = user_ty::create();
	nstring n = up->default_project();
        project_name = str_copy(n.get_ref());
    }
    else
	project_name = str_copy(project_name);
    project_ty *pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    change::pointer cp = pp->change_get();

    list_change_files(cp, args);

    project_free(pp);
    trace(("}\n"));
}
