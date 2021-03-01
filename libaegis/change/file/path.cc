//
// aegis - project change supervisor
// Copyright (C) 1999, 2000, 2002-2008, 2011, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <libaegis/change/file.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <common/str_list.h>
#include <common/symtab.h>
#include <common/trace.h>


string_ty *
change::file_path(string_ty *file_name)
{
    fstate_src_ty   *src_data;
    string_ty       *result;

    trace(("change::file_path(this = %p, file_name = \"%s\")\n{\n",
        this, file_name->str_text));
    fstate_get();
    assert(fstate_stp);
    result = 0;
    src_data = (fstate_src_ty *)fstate_stp->query(file_name);
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
            search_path_get(&search_path, false);
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

    switch (cstate_get()->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_completed:
#ifndef DEBUG
    default:
#endif
        result = project_file_path(pp, src_data);
        break;

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
        result =
            os_path_cat
            (
                change_development_directory_get(this, 0),
                src_data->file_name
            );
        break;

    case cstate_state_being_integrated:
        result =
            os_path_cat
            (
                change_integration_directory_get(this, 0),
                src_data->file_name
            );
        break;
    }

    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}


nstring
change::file_path(const nstring &file_name)
{
    string_ty *result = file_path(file_name.get_ref());
    if (!result)
        return nstring();
    return nstring(result);
}


// vim: set ts=8 sw=4 et :
