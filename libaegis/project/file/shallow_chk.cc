//
// aegis - project change supervisor
// Copyright (C) 1999, 2002-2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/project/file.h>


//
// The code in this function MUST match the code in the
// project_file_shallow function, see libaegis/project/file/shallow.c
//

int
project_file_shallow_check(project *pp, string_ty *file_name)
{
    change::pointer pcp;
    fstate_src_ty   *src_data;

    //
    // If there is no parent project,
    // nothing more needs to be done.
    //
    trace(("project_file_shallow_check(pp = %p, file_name = \"%s\")\n{\n",
        pp, file_name->str_text));
    if (pp->is_a_trunk())
    {
        trace(("shallowing \"%s\" no grandparent\n", file_name->str_text));
        trace(("return 1;\n}\n"));
        return 1;
    }

    //
    // Look for the file in the project.
    // If it is there, nothing more needs to be done.
    //
    pcp = pp->change_get();
    src_data = pcp->file_find(nstring(file_name), view_path_first);
    if (src_data)
    {
        trace(("shallowing \"%s\" already in %s\n", file_name->str_text,
            project_name_get(pp).c_str()));
        trace(("return 1;\n}\n"));
        return 1;
    }

#if 0
    //
    // The file is not part of the immediate project, see if
    // we can find it in one of the grand*parent projects.
    // If it is not there, nothing more needs to be done.
    //
    src_data = pp->file_find(file_name, view_path_simple);
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


// vim: set ts=8 sw=4 et :
