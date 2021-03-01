//
// aegis - project change supervisor
// Copyright (C) 2005, 2006, 2008, 2011, 2012 Peter Miller
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
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/project/file.h>


fstate_src_ty *
project::file_find(cstate_src_ty *c_src_data, view_path_ty vp)
{
    trace(("project::file_find(c_src = %p, vp = %s)\n{\n", c_src_data,
        view_path_ename(vp)));
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
            file_find_by_uuid(c_src_data->uuid, vp)
        :
            file_find(c_src_data->file_name, vp)
        );
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
