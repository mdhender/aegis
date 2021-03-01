//
// aegis - project change supervisor
// Copyright (C) 2001, 2003-2008, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <libaegis/change/branch.h>


string_ty *
change_branch_develop_end_notify_command_get(change::pointer cp)
{
    cstate_ty       *cstate_data;
    cstate_branch_ty *bp;

    cstate_data = cp->cstate_get();
    bp = cstate_data->branch;
    assert(bp);
    if
    (
        bp->develop_end_notify_command
    &&
        !bp->develop_end_notify_command->str_length
    )
    {
        str_free(bp->develop_end_notify_command);
        bp->develop_end_notify_command = 0;
    }
    return bp->develop_end_notify_command;
}


// vim: set ts=8 sw=4 et :
