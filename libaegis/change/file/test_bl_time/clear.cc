//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
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

#include <libaegis/change/file.h>


void
change_file_test_baseline_time_clear(change::pointer cp,
    fstate_src_ty *src_data, string_ty *variant)
{
    fstate_src_architecture_times_list_ty *atlp;
    fstate_src_architecture_times_ty *atp;
    size_t          j;

    //
    // We are clearing a test time stamp,
    // so the change summary must also be cleared
    //
    change_test_baseline_time_set(cp, (time_t)0);

    //
    // find the appropriate architecture record
    //
    atlp = src_data->architecture_times;
    if (!atlp)
	return;
    if (!variant)
	variant = change_architecture_name(cp, 1);
    for (j = 0; j < atlp->length; ++j)
    {
	atp = atlp->list[j];
	if (!atp->variant)
	    continue; // probably a bug
	if (!str_equal(atp->variant, variant))
	    continue;

	//
	// Clear the time stamp
	//
	atp->test_baseline_time = 0;
	break;
    }
}
