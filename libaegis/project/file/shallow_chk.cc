//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate shallow_chks
//

#include <change.h>
#include <change/file.h>
#include <project/file.h>
#include <trace.h>


//
// The code in this function MUST match the code in the
// project_file_shallow function, see libaegis/project/file/shallow.c
//

int
project_file_shallow_check(project_ty *pp, string_ty *file_name)
{
    change_ty       *pcp;
    fstate_src_ty   *src_data;

    //
    // If there is no parent project,
    // nothing more needs to be done.
    //
    trace(("project_file_shallow_check(pp = %08lX, file_name = \"%s\")\n{\n",
	(long)pp, file_name->str_text));
    if (!pp->parent)
    {
	trace(("shallowing \"%s\" no grandparent\n", file_name->str_text));
	trace(("return 1;\n}\n"));
	return 1;
    }

    //
    // Look for the file in the project.
    // If it is there, nothing more needs to be done.
    //
    pcp = project_change_get(pp);
    src_data = change_file_find(pcp, file_name, view_path_first);
    if (src_data)
    {
	trace(("shallowing \"%s\" already in %s\n", file_name->str_text,
	    project_name_get(pp)->str_text));
	trace(("return 1;\n}\n"));
    	return 1;
    }

#if 0
    //
    // The file is not part of the immediate project, see if
    // we can find it in one of the grand*parent projects.
    // If it is not there, nothing more needs to be done.
    //
    src_data = project_file_find(pp, file_name, view_path_simple);
    if (!src_data)
    {
	trace(("shallowing \"%s\" not in %s ancestor\n", file_name->str_text,
	    project_name_get(pp)->str_text));
	trace(("return 1;\n}\n"));
	return 1;
    }
#endif

    //
    // Oops, file is supposed to be shallow, and isn't.
    //
    trace(("return 0;\n}\n"));
    return 0;
}
