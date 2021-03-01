//
// aegis - project change supervisor
// Copyright (C) 2004-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <libaegis/change.h>
#include <libaegis/change/identifi_sub.h>
#include <libaegis/cstate.fmtgen.h>


bool
change_identifier_subset::get_devdir()
{
    if (!devdir)
    {
        cstate_ty *cstate_data = get_cp()->cstate_get();
        switch (cstate_data->state)
        {
        case cstate_state_awaiting_development:
        case cstate_state_completed:
        case cstate_state_being_integrated:
            break;

        case cstate_state_being_developed:
        case cstate_state_awaiting_review:
        case cstate_state_being_reviewed:
        case cstate_state_awaiting_integration:
            devdir = true;
            break;
        }
    }
    return devdir;
}


// vim: set ts=8 sw=4 et :
