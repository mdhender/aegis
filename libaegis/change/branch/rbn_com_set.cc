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


void
change_branch_review_begin_notify_command_set(change::pointer cp, string_ty *s)
{
    cstate_ty       *cstate_data;
    cstate_branch_ty *bp;

    cstate_data = cp->cstate_get();
    bp = cstate_data->branch;
    assert(bp);
    if (bp->review_begin_notify_command)
    {
        str_free(bp->review_begin_notify_command);
        bp->review_begin_notify_command = 0;
    }
    if (s && s->str_length)
        bp->review_begin_notify_command = str_copy(s);
}


// vim: set ts=8 sw=4 et :
