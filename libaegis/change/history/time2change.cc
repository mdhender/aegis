//
//	aegis - project change supervisor
//	Copyright (C) 2002-2007 Peter Miller
//	Copyright (C) 2007 Walter Franzini
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/trace.h>
#include <libaegis/change/branch.h>
#include <common/error.h> // for assert
#include <libaegis/project.h>


long
change_history_change_by_timestamp(project_ty *pp, time_t when)
{
    cstate_ty       *cstate_data;
    cstate_branch_history_list_ty *hl;
    change::pointer cp;

    cp = pp->change_get();
    cstate_data = cp->cstate_get();
    if (!cstate_data->branch)
	return 0;
    hl = cstate_data->branch->history;
    if (!hl)
	return 0;

    long start = 0;
    long end = hl->length - 1;

    //
    // Find the right candidate using an algorithm with a logarithmic
    // time complexity.
    //
    while (1)
    {
	change::pointer cp2;
        change::pointer cp3;

        assert (start <= end);

        trace_long(start);
        trace_long(end);

        cstate_branch_history_ty *bh_end = hl->list[end];
        cp3 = change_alloc(pp, bh_end->change_number);
        change_bind_existing(cp3);
        time_t time_end = change_completion_timestamp(cp3);
        change_free(cp3);

        if (when == time_end)
            return bh_end->change_number;

        //
        // This happend only at the 1st iteration if `when' is outside
        // the interval.
        //
        if (when > time_end)
            return bh_end->change_number;

        cstate_branch_history_ty *bh_start = hl->list[start];
        cp2 = change_alloc(pp, bh_start->change_number);
	change_bind_existing(cp2);
	time_t time_start = change_completion_timestamp(cp2);
        change_free(cp2);

        if (when == time_start)
            return bh_start->change_number;

        //
        // This happend only at the 1st iteration if `when' is outside
        // the interval.
        //
        if (when < time_start)
            return 0;

        //
        // If we cannot further reduce the interval and `when' is still
        // missing we return the oldest change (pointed by start).
        //
        if (end - start == 1)
        {
            assert (time_end > when);
            assert (when > time_start);

            return bh_start->change_number;
        }

        long middle = start + (end - start) / 2;

        cstate_branch_history_ty *bh_middle = hl->list[middle];
        change::pointer cp4 = change_alloc(pp, bh_middle->change_number);
	change_bind_existing(cp4);
	time_t time_middle = change_completion_timestamp(cp4);
        change_free(cp4);

        //
        // Avoid an unneeded loop.
        //
        if (when == time_middle)
            return bh_middle->change_number;

        if (when < time_middle)
            end = middle;
        else
            start = middle;
    }
    return 0;
}
