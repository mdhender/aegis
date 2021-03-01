//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2006, 2008 Peter Miller
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

#include <libaegis/change/file.h>
#include <common/error.h> // for assert
#include <libaegis/os/isa/path_prefix.h>
#include <libaegis/project/file.h>
#include <common/str_list.h>
#include <common/trace.h>


void
project_file_directory_query(project_ty *pp, string_ty *file_name,
    string_list_ty *result_in, string_list_ty *result_out,
    view_path_ty as_view_path)
{
    size_t          j;

    trace(("project_file_dir(pp = %8.8lX, file_name = \"%s\")\n{\n",
	(long)pp, file_name->str_text));
    assert(result_in);
    result_in->clear();
    if (result_out)
	result_out->clear();
    for (j = 0;; ++j)
    {
	fstate_src_ty   *src_data;

	src_data = pp->file_nth(j, as_view_path);
	if (!src_data)
	    break;
	switch (src_data->usage)
	{
	case file_usage_source:
	case file_usage_config:
	case file_usage_test:
	case file_usage_manual_test:
#ifndef DEBUG
	default:
#endif
	    if (os_isa_path_prefix(file_name, src_data->file_name))
	    {
		switch (src_data->action)
		{
		case file_action_remove:
		    if (result_out)
			result_out->push_back(src_data->file_name);
		    break;

		case file_action_create:
		case file_action_modify:
		case file_action_insulate:
		case file_action_transparent:
#ifndef DEBUG
		default:
#endif
		    result_in->push_back(src_data->file_name);
		    break;
		}
	    }
	    break;

	case file_usage_build:
	    break;
	}
    }
    trace(("}\n"));
}
