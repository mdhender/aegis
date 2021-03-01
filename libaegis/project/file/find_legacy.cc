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


fstate_src_ty *
project_file_find(project_ty *pp, cstate_src_ty *c_src_data, view_path_ty vp)
{
    trace(("project_file_find(pp = %08lX, c_src = %08lX, vp = %s)\n{\n",
	(long)pp, (long)c_src_data, view_path_ename(vp)));
    trace
    ((
	"change: %s %s \"%s\" %s\n", file_usage_ename(c_src_data->usage),
	file_action_ename(c_src_data->action),
	c_src_data->file_name->str_text,
	(c_src_data->edit_number ?
	    c_src_data->edit_number->str_text : "")
    ));
    fstate_src_ty *result =
	(
	    c_src_data->uuid
	?
	    pp->file_find_by_uuid(c_src_data->uuid, vp)
	:
	    project_file_find(pp, c_src_data->file_name, vp)
	);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
