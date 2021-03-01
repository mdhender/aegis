//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2006 Peter Miller;
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
// MANIFEST: functions to manipulate reg_time_sets
//

#include <libaegis/change.h>
#include <libaegis/change/architecture/find_variant.h>
#include <common/error.h> // for assert


void
change_regression_test_time_set(change_ty *cp, time_t when, string_ty *variant)
{
    if (!variant)
	variant = change_architecture_name(cp, 1);

    //
    // set the regression_test_time in the architecture variant record
    //
    assert(cp->reference_count >= 1);
    cstate_architecture_times_ty *tp =
	change_find_architecture_variant(cp, variant);
    tp->regression_test_time = when;

    //
    // set the regression_test_time in the change state.
    // figure the oldest time of all variants.
    // if one is missing, then is zero.
    //
    cstate_ty *cstate_data = change_cstate_get(cp);
    cstate_data->regression_test_time = tp->regression_test_time;
    for (size_t j = 0; j < cstate_data->architecture->length; ++j)
    {
	size_t k;
	for (k = 0; k < cstate_data->architecture_times->length; ++k)
	{
	    tp = cstate_data->architecture_times->list[k];
	    if (str_equal(cstate_data->architecture->list[j], tp->variant))
		break;
	}
	if (k >= cstate_data->architecture_times->length)
	{
	    cstate_data->regression_test_time = 0;
	    break;
	}
	if (tp->regression_test_time < cstate_data->regression_test_time)
	    cstate_data->regression_test_time = tp->regression_test_time;
    }
}
