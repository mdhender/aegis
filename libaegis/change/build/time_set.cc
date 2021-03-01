//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2008, 2012 Peter Miller
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

#include <common/now.h>
#include <common/trace.h>
#include <libaegis/change/architecture/find_variant.h>
#include <libaegis/change.h>
#include <libaegis/uname.h>


void
change_build_time_set(change::pointer cp)
{
    size_t          j, k;
    cstate_architecture_times_ty *tp;
    cstate_ty       *cstate_data;
    pconf_ty        *pconf_data;

    //
    // set the build_time in the change state.
    //
    trace(("change_build_time_set(cp = %p)\n{\n", cp));
    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 1);
    cstate_data = cp->cstate_get();
    cstate_data->build_time = now();
    if (pconf_data->build_covers_all_architectures)
    {
        //
        // set the build_time in all of the architecture variant records
        //
        for (j = 0; j < cstate_data->architecture->length; ++j)
        {
            tp =
                change_architecture_times_find
                (
                    cp,
                    cstate_data->architecture->list[j]
                );
            tp->build_time = now();
            tp->test_time = 0;
            tp->test_baseline_time = 0; // XXX
            tp->regression_test_time = 0;
            if (tp->node)
                str_free(tp->node);
            tp->node = str_copy(uname_node_get());
        }
    }
    else
    {
        //
        // set the build_time in the architecture variant record
        //
        tp = change_find_architecture_variant(cp);
        tp->build_time = now();
        tp->test_time = 0;
        tp->test_baseline_time = 0; // XXX
        tp->regression_test_time = 0;

        //
        // figure the oldest time of all variants.
        // if one is missing, then is zero.
        //
        for (j = 0; j < cstate_data->architecture->length; ++j)
        {
            for (k = 0; k < cstate_data->architecture_times->length; ++k)
            {
                tp = cstate_data->architecture_times->list[k];
                if
                (
                    str_equal
                    (
                        cstate_data->architecture->list[j],
                        tp->variant
                    )
                )
                    break;
            }
            if (k >= cstate_data->architecture_times->length)
            {
                cstate_data->build_time = 0;
                break;
            }
            if (tp->build_time < cstate_data->build_time)
                cstate_data->build_time = tp->build_time;
        }
    }
    cstate_data->test_time = 0;
    cstate_data->test_baseline_time = 0; // XXX
    cstate_data->regression_test_time = 0;
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
