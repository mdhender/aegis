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
// MANIFEST: functions to manipulate time_sets
//

#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/change/architecture/find_variant.h>


void
change_test_time_set(change_ty *cp, time_t when)
{
    trace(("change_test_time_set(cp = %08lX, when = %ld)\n{\n", (long)cp,
	(long)when));
    change_test_time_set(cp, change_architecture_name(cp, 1), when);
    trace(("}\n"));
}



void
change_test_time_set(change_ty *cp, string_ty *variant, time_t when)
{
    trace(("change_test_time_set(cp = %08lX, variant = \"%s\", when = %ld)\n"
	"{\n", (long)cp, variant->str_text, (long)when));

    //
    // set the test_time in the architecture variant record
    //
    assert(cp->reference_count >= 1);
    cstate_architecture_times_ty *tp =
	change_find_architecture_variant(cp, variant);
    trace_string(tp->variant);
    tp->test_time = when;

    //
    // set the test_time in the change state.
    // figure the oldest time of all variants.
    // if one is missing, then is zero.
    //
    cstate_ty *cstate_data = change_cstate_get(cp);
    cstate_data->test_time = when;
    if (!when)
    {
	trace(("}\n"));
	return;
    }
    for (size_t j = 0; j < cstate_data->architecture->length; ++j)
    {
	trace(("j = %ld of %ld \"%s\"\n", (long)j,
	    (long)cstate_data->architecture->length,
	    cstate_data->architecture->list[j]->str_text));
	size_t k = 0;
	for (k = 0; k < cstate_data->architecture_times->length; ++k)
	{
	    tp = cstate_data->architecture_times->list[k];
	    trace(("k = %ld of %ld \"%s\"\n", (long)k,
		(long)cstate_data->architecture_times->length,
		tp->variant->str_text));
	    if (str_equal(cstate_data->architecture->list[j], tp->variant))
		break;
	}
	if (k >= cstate_data->architecture_times->length)
	{
	    trace(("clear change test time\n"));
	    cstate_data->test_time = 0;
	    break;
	}
	if (tp->test_time < cstate_data->test_time)
	{
	    trace(("shorten change test time\n"));
	    cstate_data->test_time = tp->test_time;
	    trace_time(cstate_data->test_time);
	    if (!cstate_data->test_time)
		break;
	}
    }
    trace(("}\n"));
}
