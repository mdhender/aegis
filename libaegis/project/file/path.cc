//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate paths
//

#include <change/file.h>
#include <error.h>
#include <project/file.h>
#include <trace.h>


string_ty *
project_file_path(project_ty *pp, string_ty *file_name)
{
    project_ty      *ppp;

    trace(("project_file_path(pp = %8.8lX, file_name = \"%s\")\n{\n",
	    (long)pp, file_name->str_text));
    for (ppp = pp; ppp; ppp = ppp->parent)
    {
	change_ty	*cp;
	fstate_src_ty   *src_data;
	string_ty	*result;

	cp = project_change_get(ppp);
	src_data = change_file_find(cp, file_name, view_path_first);
	if (!src_data)
	    continue;
	switch (src_data->action)
	{
	case file_action_transparent:
	    continue;

	case file_action_remove:
	    break;

	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
#ifndef DEBUG
	default:
#endif
	    // should be file_action_remove
	    assert(!src_data->deleted_by);
	    if (src_data->deleted_by)
		break;

	    // should be file_action_transparent
	    assert(!src_data->about_to_be_copied_by);
	    if (src_data->about_to_be_copied_by)
		continue;

	    // should be file_action_transparent
	    assert(!src_data->about_to_be_created_by);
	    if (src_data->about_to_be_created_by)
		continue;
	    break;
	}
	result = change_file_path(cp, file_name);
	assert(result);
	assert(result->str_text[0] == '/');
	trace(("return \"%s\";\n", result->str_text));
	trace(("}\n"));
	return result;
    }
    this_is_a_bug();
    trace(("return NULL;\n"));
    trace(("}\n"));
    return 0;
}
