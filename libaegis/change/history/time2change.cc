//
//      aegis - project change supervisor
//      Copyright (C) 2002-2008, 2011, 2012 Peter Miller
//      Copyright (C) 2007, 2008 Walter Franzini
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/change/branch.h>
#include <libaegis/project.h>


long
change_history_change_by_timestamp(project *pp, time_t when)
{
    trace(("%s\n{\n", __PRETTY_FUNCTION__));
    cstate_ty       *cstate_data;
    cstate_branch_history_list_ty *hl;
    change::pointer cp;
    long result = 0;

    cp = pp->change_get();
    cstate_data = cp->cstate_get();
    if (!cstate_data->branch)
        return 0;
    hl = cstate_data->branch->history;
    if (!hl)
        return 0;

    trace(("hl->length = %ld;\n", (unsigned long)hl->length));
    if (hl->length == 0)
        return 0;

    assert(hl->list);
    if (!hl->list)
        return 0;

    long start = 0;
    long end = hl->length - 1;

    cstate_branch_history_ty *bh_start = hl->list[start];
    assert(bh_start);
    time_t time_start =
        pp->change_completion_timestamp(bh_start->change_number);

    cstate_branch_history_ty *bh_end = hl->list[end];
    assert(bh_end);
    time_t time_end =
        pp->change_completion_timestamp(bh_end->change_number);

    //
    // Find the right candidate using an algorithm with a logarithmic
    // time complexity.
    //
    for (;;)
    {
        assert (start <= end);

        trace_long(start);
        trace_long(end);

        if (when == time_end)
        {
            assert(hl->list[end]);
            result = hl->list[end]->change_number;
            break;
        }

        //
        // This happens only at the first iteration if `when' is outside
        // the interval.
        //
        if (when > time_end)
        {
            assert(hl->list[end]);
            result = hl->list[end]->change_number;
            break;
        }

        if (when == time_start)
        {
            assert(hl->list[start]);
            result = hl->list[start]->change_number;
            break;
        }

        //
        // This happens only at the first iteration if `when' is outside
        // the interval.
        //
        if (when < time_start)
        {
            result = 0;
            break;
        }

        //
        // If we cannot further reduce the interval and `when' is still
        // missing we return the oldest change (pointed by start).
        //
        if (end - start == 1)
        {
            assert (time_end > when);
            assert (when > time_start);
            assert(hl->list[start]);

            result = hl->list[start]->change_number;
            break;
        }

        long middle = (start + end) / 2;

        cstate_branch_history_ty *bh_middle = hl->list[middle];
        assert(bh_middle);
        time_t time_middle =
            bh_middle->when != TIME_NOT_SET
            ? bh_middle->when
            : pp->change_completion_timestamp(bh_middle->change_number);
        if (when < time_middle)
        {
            time_end = time_middle;
            end = middle;
        }
        else
        {
            time_start = time_middle;
            start = middle;
        }
    }

    return result;
}


// vim: set ts=8 sw=4 et :
