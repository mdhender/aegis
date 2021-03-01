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

#include <common/trace.h>
#include <libaegis/change/file.h>


void
change_file_test_baseline_time_set(change::pointer cp, fstate_src_ty *src_data,
    time_t when, string_ty *variant)
{
    trace(("change_file_test_baseline_time_set(cp = %p, src_data = %p, "
        "when = %ld, variant = \"%s\")\n{\n", cp, src_data,
        (long)when, variant->str_text));
    fstate_src_architecture_times_ty *atp = 0;
    size_t          j;
    size_t          k;

    //
    // Find the appropriate architecture record;
    // create a new one if necessary.
    //
    if (!variant)
        variant = change_architecture_name(cp, 1);
    trace_string(variant);
    if (!src_data->architecture_times)
    {
        src_data->architecture_times =
            (fstate_src_architecture_times_list_ty *)
            fstate_src_architecture_times_list_type.alloc();
    }
    fstate_src_architecture_times_list_ty *atlp = src_data->architecture_times;
    for (j = 0; j < atlp->length; ++j)
    {
        atp = atlp->list[j];
        if
        (
            // bug if not set
            atp->variant
        &&
            str_equal(atp->variant, variant)
        )
                break;
    }
    if (j >= atlp->length)
    {
        fstate_src_architecture_times_ty **addr;
        meta_type *type_p = 0;

        addr =
            (fstate_src_architecture_times_ty **)
            fstate_src_architecture_times_list_type.list_parse(atlp, &type_p);
        assert(type_p == &fstate_src_architecture_times_type);
        atp =
            (fstate_src_architecture_times_ty *)
            fstate_src_architecture_times_type.alloc();
        *addr = atp;
        atp->variant = str_copy(variant);
    }

    //
    // Remember the test time.
    //
    assert(atp);
    assert(when);
    atp->test_baseline_time = when;

    //
    // We need to make sure that the change summary reflects whether
    // or not all tests have been run for this architecture.
    //
    for (j = 0; ; ++j)
    {
        src_data = change_file_nth(cp, j, view_path_first);
        if (!src_data)
            break;
        switch (src_data->usage)
        {
        case file_usage_test:
        case file_usage_manual_test:
            switch (src_data->action)
            {
            case file_action_remove:
            case file_action_modify:
            case file_action_transparent:
                    continue;

            case file_action_create:
            case file_action_insulate:
                // should be file_action_removed
                assert(!src_data->deleted_by);
                if (src_data->deleted_by)
                    continue;

                // should be file_action_transparent
                assert(!src_data->about_to_be_created_by);
                if (src_data->about_to_be_created_by)
                    continue;
                break;
            }
            break;

        case file_usage_source:
        case file_usage_config:
        case file_usage_build:
            continue;
        }

        atlp = src_data->architecture_times;
        if (!atlp)
        {
            // All architectures missing.
            when = 0;
            break;
        }
        for (k = 0; k < atlp->length; ++k)
        {
            atp = atlp->list[k];
            if
            (
                // bug if not set
                atp->variant
            &&
                str_equal(atp->variant, variant)
            )
            {
                if (atp->test_baseline_time < when)
                    when = atp->test_baseline_time;
                break;
            }
        }
        if (k >= atlp->length)
        {
            // Found a missing architecture.
            when = 0;
            break;
        }
    }

    //
    // set the change test time
    //
    change_test_baseline_time_set(cp, variant, when);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
