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
 * MANIFEST: functions to manipulate querys
 */

#include <change/file.h>
#include <error.h> /* for assert */
#include <os/isa/path_prefix.h>
#include <project/file.h>
#include <str_list.h>
#include <trace.h>


void
project_file_directory_query(project_ty *pp, string_ty *file_name,
    string_list_ty *result_in, string_list_ty *result_out,
    view_path_ty as_view_path)
{
    size_t          j;

    trace(("project_file_dir(pp = %8.8lX, file_name = \"%s\")\n{\n",
	(long)pp, file_name->str_text));
    assert(result_in);
    string_list_constructor(result_in);
    if (result_out)
	string_list_constructor(result_out);
    for (j = 0;; ++j)
    {
	fstate_src	src_data;

	src_data = project_file_nth(pp, j, as_view_path);
	if (!src_data)
	    break;
	if (src_data->usage == file_usage_build)
	    continue;
	if (os_isa_path_prefix(file_name, src_data->file_name))
	{
	    if (src_data->action != file_action_remove)
	       	string_list_append(result_in, src_data->file_name);
	    else if (result_out)
	       	string_list_append(result_out, src_data->file_name);
	}
    }
    trace(("}\n"));
}
