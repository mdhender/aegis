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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//
// MANIFEST: functions to list state file names
//

#include <common/ac/stdio.h>

#include <libaegis/ael/state_file.h>
#include <libaegis/change.h>
#include <libaegis/project.h>
#include <common/str_list.h>
#include <libaegis/user.h>


void
list_state_file_name(string_ty *project_name, long change_number,
    string_list_ty *)
{
	project_ty	*pp;
	change::pointer cp;

	//
	// locate project data
	//
	if (!project_name)
        {
                nstring n = user_ty::create()->default_project();
                project_name = str_copy(n.get_ref());
        }
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	pp->bind_existing();

	//
	// locate change data
	//
	if (!change_number)
		cp = pp->change_get();
	else
	{
		cp = change_alloc(pp, change_number);
		change_bind_existing(cp);
	}

	//
	// Print out the state file's path.
	//
	printf("%s\n", change_cstate_filename_get(cp)->str_text);
}
