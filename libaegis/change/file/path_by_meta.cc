//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: implementation of the change_file_path_by_meta class
//

#include <libaegis/change/file.h>
#include <common/trace.h>


string_ty *
change_file_path(change_ty *cp, fstate_src_ty *src)
{
    trace(("change_file_path(cp = %08lX, src = \"%08lX\")\n{\n", (long)cp,
	(long)src));
    string_ty *result =
	(
	    src->uuid
	?
	    change_file_path_by_uuid(cp, src->uuid)
	:
	    change_file_path(cp, src->file_name)
	);
    trace(("return \"%s\";\n", (result ? result->str_text : "")));
    trace(("}\n"));
    return result;
}
