//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2003-2008, 2012 Peter Miller
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
#include <libaegis/change.h>
#include <libaegis/change/architecture/find_variant.h>


void
change_test_time_set(change::pointer cp, time_t when)
{
    trace(("change_test_time_set(cp = %p, when = %ld)\n{\n", cp, when));
    change_test_time_set(cp, change_architecture_name(cp, 1), when);
    trace(("}\n"));
}



void
change_test_time_set(change::pointer cp, string_ty *variant, time_t when)
{
    trace(("change_test_time_set(cp = %p, variant = \"%s\", when = %ld)\n"
        "{\n", cp, variant->str_text, (long)when));

    trace(("change_test_time_set(cp = %p)\n{\n", cp));
    assert(cp->reference_count >= 1);
    pconf_ty *pconf_data = change_pconf_get(cp, 1);
    cstate_ty *cstate_data = cp->cstate_get();
    // set the test_time in the change state - no matter what
    cstate_data->test_time = when;

    if (pconf_data->test_covers_all_architectures)
    {
        //
        // set the test_time in all of the architecture variant records
        //
        for (size_t j = 0; j < cstate_data->architecture->length; ++j)
        {
            trace(("j = %ld of %ld \"%s\"\n", (long)j,
                (long)cstate_data->architecture->length,
                cstate_data->architecture->list[j]->str_text));

            cstate_architecture_times_ty *tp =
                cstate_data->architecture_times->list[j];
            trace(("j = %ld of %ld \"%s\"\n", (long)j,
                   (long)cstate_data->architecture_times->length,
                   tp->variant->str_text));
            tp->test_time = when;
        }
    }
    else
    {
        //
        // set the test_time in the one architecture variant record
        //
        cstate_architecture_times_ty *tp =
            change_find_architecture_variant(cp, variant);
        trace_string(tp->variant);
        tp->test_time = when;

        if (when)
        {
            // figure the oldest time of all variants.
            // if one is missing, then is zero.
            // TODO: why traverse the list nested? Once is enough yes?
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
                    if (str_equal(cstate_data->architecture->list[j],
                                  tp->variant))
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
        }
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
