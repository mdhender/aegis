//
//      aegis - project change supervisor
//      Copyright (C) 2005-2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/change/file.h>
#include <libaegis/project/file.h>


string_ty *
project_file_path_by_uuid(project *pp, string_ty *uuid)
{
    trace(("project_file_path_by_uuid(pp = %p, uuid = \"%s\")\n{\n",
        pp, uuid->str_text));
    project *ppp = pp;
    for (; ppp; ppp = (ppp->is_a_trunk() ? 0 : ppp->parent_get()))
    {
        change::pointer cp = ppp->change_get();
        fstate_src_ty *src_data =
            cp->file_find_uuid(uuid, view_path_first);
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
        string_ty *result = cp->file_path_by_uuid(uuid);
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


// vim: set ts=8 sw=4 et :
