//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2005 Peter Miller;
//	All rights reserved.
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
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate times_clears
//

#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <common/error.h> // for assert


void
change_build_times_clear(change_ty *cp)
{
    cstate_ty       *cstate_data;
    size_t          j;

    //
    // reset the build and test times in all architecture variant records
    //
    assert(cp->reference_count >= 1);
    cstate_data = change_cstate_get(cp);
    if (cstate_data->architecture_times)
    {
	cstate_architecture_times_list_type.free
	(
    	    cstate_data->architecture_times
	);
	cstate_data->architecture_times = 0;
    }

    //
    // reset the test times in the change state
    //
    cstate_data->build_time = 0;
    cstate_data->test_time = 0;
    cstate_data->test_baseline_time = 0; // XXX
    cstate_data->regression_test_time = 0;

    //
    // reset file test times
    //
    for (j = 0; ; ++j)
    {
	fstate_src_ty   *src_data;

	src_data = change_file_nth(cp, j, view_path_first);
	if (!src_data)
	    break;
	if (src_data->architecture_times)
	{
	    fstate_src_architecture_times_list_type.free
	    (
		src_data->architecture_times
	    );
	    src_data->architecture_times = 0;
	}
    }
}
