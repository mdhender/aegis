//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2007 Peter Miller
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
// MANIFEST: functions to manipulate news
//

#include <libaegis/change/file.h>
#include <libaegis/project/file.h>
#include <common/trace.h>


fstate_src_ty *
project_ty::file_new(string_ty *file_name)
{
    trace(("project_ty::file_new(this = %8.8lX, file_name = \"%s\")\n{\n",
	(long)this, file_name->str_text));
    fstate_src_ty *src_data = change_get()->file_new(file_name);
    trace(("return %8.8lX;\n", (long)src_data));
    trace(("}\n"));
    return src_data;
}


fstate_src_ty *
project_ty::file_new(fstate_src_ty *meta)
{
    trace(("project_ty::file_new(this = %8.8lX, meta->file_name = \"%s\")\n{\n",
	(long)this, meta->file_name->str_text));
    fstate_src_ty *src_data = change_get()->file_new(meta);
    trace(("return %8.8lX;\n", (long)src_data));
    trace(("}\n"));
    return src_data;
}
