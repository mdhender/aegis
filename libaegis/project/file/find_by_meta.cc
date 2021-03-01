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
// MANIFEST: functions to manipulate find_by_metas
//

#include <error.h> // for assert
#include <project/file.h>
#include <trace.h>


fstate_src_ty *
project_file_find_by_meta(project_ty *pp, fstate_src_ty *c_src_data,
    view_path_ty vp)
{
    fstate_src_ty   *p_src_data;
    string_ty       *name;

    trace(("project_file_find_by_meta(pp = %08lX, c_src = %08lX, vp = %s)\n{\n",
	(long)pp, (long)c_src_data, view_path_ename(vp)));
    trace
    ((
	"change: %s %s \"%s\" %s %s\n", file_usage_ename(c_src_data->usage),
	file_action_ename(c_src_data->action),
	c_src_data->file_name->str_text,
	(c_src_data->edit_origin ?
	    c_src_data->edit_origin->revision->str_text : ""),
	(c_src_data->edit ?
	    c_src_data->edit->revision->str_text : "")
    ));

    //
    // Try to find the file by its UUID.
    //
    // For backwards compatibility reasons (UUIDs were introduced in
    // 4.17) there could be a rename chain (pointing at UUID-less
    // entries) to be taken into account.
    //
    if (c_src_data->uuid)
    {
	p_src_data = project_file_find_by_uuid(pp, c_src_data->uuid, vp);
	while (p_src_data)
	{
	    if (p_src_data->action != file_action_remove || !p_src_data->move)
	    {
		trace
		((
		    "project: %s %s \"%s\" %s %s\n",
		    file_usage_ename(p_src_data->usage),
		    file_action_ename(p_src_data->action),
		    p_src_data->file_name->str_text,
		    (p_src_data->edit_origin ?
			p_src_data->edit_origin->revision->str_text : ""),
		    (p_src_data->edit ?
			p_src_data->edit->revision->str_text : "")
		));
		trace(("return %08lX;\n", (long)p_src_data));
		trace(("}\n"));
		return p_src_data;
	    }
	    name = p_src_data->move;
	    p_src_data = project_file_find(pp, name, vp);
	}
    }

    //
    // Look for the file by name, tracking renames as they happen.
    //
    if (c_src_data->action == file_action_create && c_src_data->move)
	name = c_src_data->move;
    else
	name = c_src_data->file_name;
    for (;;)
    {
	p_src_data = project_file_find(pp, name, vp);
	if (!p_src_data)
	{
	    trace(("return NULL;\n"));
	    trace(("}\n"));
	    return 0;
	}
	if (p_src_data->action != file_action_remove || !p_src_data->move)
	{
	    trace
	    ((
		"project: %s %s \"%s\" %s %s\n",
		file_usage_ename(p_src_data->usage),
		file_action_ename(p_src_data->action),
		p_src_data->file_name->str_text,
		(p_src_data->edit_origin ?
		    p_src_data->edit_origin->revision->str_text : ""),
		(p_src_data->edit ?
		    p_src_data->edit->revision->str_text : "")
	    ));
	    trace(("return %08lX;\n", (long)p_src_data));
	    trace(("}\n"));
	    return p_src_data;
	}
	name = p_src_data->move;
    }
}
