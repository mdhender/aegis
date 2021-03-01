//
// aegis - project change supervisor
// Copyright (C) 2007 Walter Franzini
// Copyright (C) 2008, 2011, 2012 Peter Miller
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

#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/project.h>
#include <libaegis/view_path/next_change.h>

change::pointer
view_path_next_change(change::pointer cp, time_t limit)
{
    //
    // Select the next change to consult:
    //
    // 1. if the time limit is not set we use the parent project
    //    corresponding change.  This is the old Aegis behaviour.
    // 2. if the time limit is set we need to find the change
    //    current at the time limit points to.  This is
    //    complicated a bit by the fact that the pfstate data may
    //    be missing.
    //
    if (limit == TIME_NOT_SET)
        return change_copy(cp->pp->change_get());

    project *pp;
    if (cp->pfstate_get() && !cp->pp->is_a_trunk())
        pp = cp->pp->parent_get();
    else
        pp = cp->pp;

    long change_number =
        change_history_change_by_timestamp(pp, limit);

    if (!change_number)
        return change_copy(pp->change_get());

    change::pointer result;
    change::pointer cp_tmp = change_alloc(pp, change_number);
    change_bind_existing(cp_tmp);
    if (cp_tmp->pfstate_get())
    {
        result = cp_tmp;
    }
    else
    {
        change_free(cp_tmp);
        result = change_copy(pp->change_get());
    }

    return result;
}


// vim: set ts=8 sw=4 et :
