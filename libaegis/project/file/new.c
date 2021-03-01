/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate news
 */

#include <change/file.h>
#include <project/file.h>
#include <trace.h>


fstate_src
project_file_new(project_ty *pp, string_ty *file_name)
{
    change_ty       *cp;
    fstate_src      src_data;

    trace(("project_file_new(pp = %8.8lX, file_name = \"%s\")\n{\n",
	(long)pp, file_name->str_text));
    cp = project_change_get(pp);
    src_data = change_file_new(cp, file_name);
    trace(("return %8.8lX;\n", (long)src_data));
    trace(("}\n"));
    return src_data;
}
