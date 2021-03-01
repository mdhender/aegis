//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: functions to manipulate path_gets
//

#include <project.h>
#include <trace.h>


string_ty *
project_history_path_get(project_ty *pp)
{
    trace(("project_history_path_get(pp = %08lX)\n{\n", (long)pp));
    while (pp->parent)
	pp = pp->parent;
    if (!pp->history_path)
    {
	pp->history_path =
	    str_format("%s/history", project_home_path_get(pp)->str_text);
    }
    trace(("return \"%s\";\n", pp->history_path->str_text));
    trace(("}\n"));
    return pp->history_path;
}
