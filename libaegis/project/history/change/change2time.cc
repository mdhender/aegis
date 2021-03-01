//
// aegis - project change supervisor
// Copyright (C) 2007, 2008 Walter Franzini
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

#include <common/ac/assert.h>

#include <common/itab.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/project.h>


time_t
project::change_completion_timestamp(long change_number)
{
    assert(change2time_stp);
    time_t result = (time_t)itab_query(change2time_stp, change_number);
    if (!result)
    {
        //
        // Scan the history
        //
        change::pointer cp2 = change_get();
        cstate_ty *cstate_data = cp2->cstate_get();
        if (cstate_data->branch && cstate_data->branch->history)
        {
            cstate_branch_history_list_ty *hl = cstate_data->branch->history;
            for (size_t j = 0; j < hl->length; ++j)
            {
                if (hl->list[j]->change_number != change_number)
                    continue;
                if (hl->list[j]->when == TIME_NOT_SET)
                    break;

                itab_assign
                (
                    change2time_stp,
                    change_number,
                    (void*)hl->list[j]->when
                );
                return hl->list[j]->when;
            }
        }

        change::pointer cp = change_alloc(this, change_number);
        change_bind_existing(cp);
        result = cp->completion_timestamp();
        itab_assign (change2time_stp, cp->number, (void*)result);
        change_free(cp);
    }

    return result;
}


// vim: set ts=8 sw=4 et :
