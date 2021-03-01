//
// aegis - project change supervisor
// Copyright (C) 2008, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/trace.h>

#include <libaegis/change/file.h>
#include <libaegis/change/functor.h>
#include <libaegis/change/identifi_sub.h>
#include <libaegis/file/event.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/sub.h>


file_revision
change_identifier_subset::get_file_revision(fstate_src_ty *orig_src,
    change_functor &bad_state)
{
    //
    // Check the change state.
    //
    trace(("change_identifier_subset::get_file_revision_by_uuid(\"%s\")\n{\n",
        orig_src->file_name->str_text));
    assert(orig_src);
    get_cp();
    assert(cp);
    cstate_ty *cstate_data = cp->cstate_get();
    fstate_src_ty *result_src = 0;
    switch (cstate_data->state)
    {
#ifndef DEBUG
    default:
#endif
    case cstate_state_awaiting_development:
        bad_state(cp);
        trace(("}\n"));
        return file_revision("/dev/null", false);

    case cstate_state_completed:
        {
            //
            // Need to reconstruct the appropriate file histories.
            //
            trace(("project = \"%s\"\n",
                project_name_get(pid.get_pp()).c_str()));
            project_file_roll_forward *hp = get_historian();
            file_event *fep = hp->get_last(orig_src);
            if (!fep)
            {
                //
                // The file doesn't exist yet at this
                // delta.  Omit it.
                //
                trace(("}\n"));
                return file_revision("/dev/null", false);
            }
            result_src = fep->get_src();
        }
        break;

    case cstate_state_being_integrated:
    case cstate_state_awaiting_integration:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_review:
    case cstate_state_being_developed:
        trace(("mark\n"));
        result_src = cp->file_find(orig_src, view_path_simple);
        if (!result_src)
        {
            sub_context_ty sc;
            sc.var_set_string("File_Name", orig_src->file_name);
            change_fatal(cp, &sc, i18n("no $filename"));
        }
        break;
    }
    trace(("result_src = %p\n", result_src));
    assert(result_src);
    switch (result_src->action)
    {
    case file_action_remove:
        //
        // The file had been removed at this
        // delta.  Omit it.
        //
        trace(("}\n"));
        return file_revision("/dev/null", false);

    case file_action_create:
    case file_action_modify:
    case file_action_insulate:
    case file_action_transparent:
        break;
    }
    int from_unlink = 0;
    trace(("edit = %s\n", result_src->edit->revision->str_text));
    nstring from(change_file_version_path(cp, result_src, &from_unlink));
    trace(("}\n"));
    return file_revision(from, from_unlink);
}


// vim: set ts=8 sw=4 et :
