//
// aegis - project change supervisor
// Copyright (C) 2002-2008, 2012 Peter Miller
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

#include <common/now.h>
#include <libaegis/change.h>


time_t
change::completion_timestamp(void)
{
    cstate_ty *csd = cstate_get();
    if (!bogus && csd->state == cstate_state_completed)
    {
        cstate_history_list_ty *chlp = csd->history;
        assert(chlp);
        assert(chlp->length);
        cstate_history_ty *chp = chlp->list[chlp->length - 1];
        assert(chp);
        return chp->when;
    }
    return now();
}


// vim: set ts=8 sw=4 et :
