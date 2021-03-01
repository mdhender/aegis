//
// aegis - project change supervisor
// Copyright (C) 2005, 2006, 2008, 2010-2012 Peter Miller
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

#include <common/mem.h>
#include <common/trace.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/identifi_sub.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/sub.h>


project_file_roll_forward *
change_identifier_subset::get_historian(bool detailed)
{
    //
    // Need to reconstruct the appropriate file histories.
    //
    trace(("project = \"%s\"\n", project_name_get(pid.get_pp()).c_str()));
    if (!historian_p)
    {
        historian_p =
            new project_file_roll_forward
            (
                pid.get_pp(),
                (
                    delta_date != NO_TIME_SET
                ?
                    delta_date
                :
                    get_cp()->completion_timestamp()
                ),
                detailed
            );
        trace_pointer(historian_p);

        cstate_ty *cstate_data = get_cp()->cstate_get();
        switch (cstate_data->state)
        {
        case cstate_state_awaiting_development:
#ifndef DEBUG
        default:
#endif
            change_fatal(get_cp(), 0, i18n("bad send state"));

        case cstate_state_completed:
            //
            // Need to reconstruct the appropriate file histories.
            //
            historian_p->set
            (
                get_pp(),
                (
                    (delta_date != NO_TIME_SET)
                ?
                    delta_date
                :
                    get_cp()->completion_timestamp()
                ),
                0
            );
            break;

        case cstate_state_being_integrated:
        case cstate_state_awaiting_integration:
        case cstate_state_being_reviewed:
        case cstate_state_awaiting_review:
        case cstate_state_being_developed:
            break;
        }
    }
    return historian_p;
}


// vim: set ts=8 sw=4 et :
