//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
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

#include <common/trace.h>
#include <libaegis/cstate.h>
#include <libaegis/project/file.h>


string_ty *
project_file_path(project_ty *pp, cstate_src_ty *src)
{
    trace(("project_file_path(pp = %8.8lX, src = %08lX)\n{\n", (long)pp,
	(long)src));
    string_ty *result =
	(
	    src->uuid
	?
	    project_file_path_by_uuid(pp, src->uuid)
	:
	    project_file_path(pp, src->file_name)
	);
    trace(("return \"%s\";\n", (result ? result->str_text : "")));
    trace(("}\n"));
    return result;
}
