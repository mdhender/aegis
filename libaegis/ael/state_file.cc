//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to list state file names
//

#include <ac/stdio.h>

#include <ael/state_file.h>
#include <change.h>
#include <project.h>
#include <str_list.h>
#include <user.h>


void
list_state_file_name(string_ty *project_name,
                     long change_number,
                     string_list_ty *args)
{
	project_ty	*pp;
	change_ty	*cp;

	//
	// locate project data
	//
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	//
	// locate change data
	//
	if (!change_number)
		cp = project_change_get(pp);
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
