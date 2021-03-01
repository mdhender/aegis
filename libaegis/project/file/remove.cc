//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2004-2006 Peter Miller;
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
// MANIFEST: functions to manipulate removes
//

#include <libaegis/change/file.h>
#include <libaegis/project/file.h>
#include <common/trace.h>


void
project_file_remove(project_ty *pp, string_ty *file_name)
{
    change_ty       *cp;

    trace(("project_file_remove(pp = %8.8lX, file_name = \"%s\")\n{\n",
	(long)pp, file_name->str_text));
    cp = pp->change_get();
    change_file_remove(cp, file_name);
    trace(("}\n"));
}
