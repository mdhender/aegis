//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <common/str_list.h>
#include <common/symtab.h>
#include <common/trace.h>


string_ty *
change_file_path_by_uuid(change::pointer cp, string_ty *uuid)
{
    trace(("change_file_path_by_uuid(cp = %08lX, uuid = \"%s\")\n{\n",
	(long)cp, uuid->str_text));
    change_fstate_get(cp);
    assert(cp->fstate_stp);
    string_ty *result = 0;
    fstate_src_ty *src_data =
	(fstate_src_ty *)symtab_query(cp->fstate_uuid_stp, uuid);
    if (!src_data)
    {
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }
    if (src_data->about_to_be_copied_by)
    {
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    //
    // Files which are built could be any where in the change search
    // path.  Go hunting for such files.
    //
    switch (src_data->usage)
    {
    case file_usage_source:
    case file_usage_config:
    case file_usage_test:
    case file_usage_manual_test:
	break;

    case file_usage_build:
	{
	    string_list_ty search_path;
	    change_search_path_get(cp, &search_path, 0);
	    assert(search_path.nstrings >= 1);
	    result = 0;
	    os_become_orig();
	    size_t j;
	    for (j = 0; j < search_path.nstrings; ++j)
	    {
		result =
		    os_path_cat(search_path.string[j], src_data->file_name);
		if (os_exists(result))
		    break;
		str_free(result);
		result = 0;
	    }
	    os_become_undo();
	    if (j >= search_path.nstrings)
	    {
		result =
		    os_path_cat(search_path.string[0], src_data->file_name);
	    }
	    assert(result);
	    trace(("return \"%s\";\n", result->str_text));
	    trace(("}\n"));
	    return result;
	}
    }

    cstate_ty *cstate_data = cp->cstate_get();
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_completed:
#ifndef DEBUG
    default:
#endif
	result = project_file_path(cp->pp, src_data);
	break;

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
	result =
    	    os_path_cat
	    (
		change_development_directory_get(cp, 0),
		src_data->file_name
	    );
	break;

    case cstate_state_being_integrated:
	result =
    	    os_path_cat
	    (
		change_integration_directory_get(cp, 0),
		src_data->file_name
	    );
	break;
    }

    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}
