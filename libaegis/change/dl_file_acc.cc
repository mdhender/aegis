//
// aegis - project change supervisor
// Copyright (C) 2011 Peter Miller
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

#include <common/ac/unistd.h>

#include <libaegis/change.h>
#include <libaegis/glue.h>
#include <libaegis/os.h>

bool
change::download_files_accessable(void)
{
    switch (cstate_get()->state)
    {
    case cstate_state_awaiting_development:
        return true;

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
        {
            change_become(this);
            nstring devdir(change_development_directory_get(this, false));
            change_become_undo(this);

            os_become_orig();
            bool result = glue_access(devdir.c_str(), R_OK | X_OK) >= 0;
            os_become_undo();

            return result;
        }

    case cstate_state_being_integrated:
        {
            change_become(this);
            nstring intdir(change_integration_directory_get(this, false));
            change_become_undo(this);

            os_become_orig();
            bool result = glue_access(intdir.c_str(), R_OK | X_OK) >= 0;
            os_become_undo();

            return result;
        }

    case cstate_state_completed:
        return true;

    default:
        return false;
    }
}


// vim: set ts=8 sw=4 et :
