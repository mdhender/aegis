//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
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

#include <common/error.h>

#include <libaegis/change/identifi_sub.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


void
change_identifier_subset::set_delta_from_branch_head(void)
{
    assert(!set());
    const cstate_ty *cstate_data = get_pp()->change_get()->cstate_get();
    assert(cstate_data->branch);
    const cstate_branch_history_list_ty *lp = cstate_data->branch->history;
    if (!lp || !lp->length)
    {
        project_fatal(get_pp(), 0, i18n("no delta yet"));
        //NOTREACHED
    }
    const cstate_branch_history_ty *hp = lp->list[lp->length - 1];
    change_number = hp->change_number;
    delta_number = hp->delta_number;
}


// vim: set ts=8 sw=4 et :
