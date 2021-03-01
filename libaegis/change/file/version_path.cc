//
// aegis - project change supervisor
// Copyright (C) 2004-2008, 2012 Peter Miller
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

#include <libaegis/change/file.h>
#include <libaegis/project/file.h>
#include <common/trace.h>


string_ty *
change_file_version_path(change::pointer cp, fstate_src_ty *src,  int *unlink_p)
{
    trace(("change_file_version_path(cp = %p, src = %p)\n{\n", cp, src));
    fstate_src_trace(src);
    if (!cp->bogus)
    {
        cstate_ty *cstate_data = cp->cstate_get();
        switch (cstate_data->state)
        {
        case cstate_state_awaiting_development:
            break;

        case cstate_state_being_developed:
        case cstate_state_awaiting_review:
        case cstate_state_being_reviewed:
        case cstate_state_awaiting_integration:
        case cstate_state_being_integrated:
            trace(("looking in change...\n"));
            if (cp->file_find(nstring(src->file_name), view_path_first))
            {
                // Note: it could be transparent.
                string_ty *rslt = cp->file_path(src->file_name);
                if (rslt)
                {
                    *unlink_p = 0;
                    trace(("return %p;\n", rslt));
                    trace(("}\n"));
                    return rslt;
                }
            }
            break;

        case cstate_state_completed:
            break;
        }
    }
    trace(("looking in project...\n"));
    string_ty *rslt = project_file_version_path(cp->pp, src, unlink_p);
    trace(("return %p;\n", rslt));
    trace(("}\n"));
    return rslt;
}

// vim: set ts=8 sw=4 et :
