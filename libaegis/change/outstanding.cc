//
// aegis - project change supervisor
// Copyright (C) 1999, 2002-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/str_list.h>
#include <libaegis/change.h>
#include <libaegis/sub.h>


static char *
outstanding_commentary(const string_list_ty &wl, long narch)
{
    static string_ty *s;

    if (s)
    {
        str_free(s);
        s = 0;
    }
    if (!wl.nstrings || wl.nstrings >= (size_t)narch)
        s = str_from_c("");
    else
    {
        sub_context_ty  *scp;
        string_ty       *t1;
        string_ty       *t2;
        size_t          j;

        t1 = str_format("\"%s\"", wl.string[0]->str_text);
        for (j = 1; j < wl.nstrings; ++j)
        {
            scp = sub_context_new();
            sub_var_set_string(scp, "Name1", t1);
            sub_var_set_format
            (
                scp,
                "Name2",
                "\"%s\"",
                wl.string[j]->str_text
            );
            if (j == wl.nstrings - 1)
                t2 = subst_intl(scp, i18n("$name1 and $name2"));
            else
                t2 = subst_intl(scp, i18n("$name1, $name2"));
            sub_context_delete(scp);
            str_free(t1);
            t1 = t2;
        }
        scp = sub_context_new();
        sub_var_set_string(scp, "Name_List", t1);
        sub_var_set_long(scp, "Number", (long)wl.size());
        sub_var_optional(scp, "Number");
        s = subst_intl(scp, i18n("for the $name_list architectures"));
        sub_context_delete(scp);
        str_free(t1);
    }
    return s->str_text;
}


const char *
change_outstanding_builds(change::pointer cp, time_t t)
{
    cstate_ty       *cstate_data;
    cstate_architecture_times_ty *tp;
    size_t          j;

    assert(cp->reference_count >= 1);
    cstate_data = cp->cstate_get();
    string_list_ty wl;
    for (j = 0; j < cstate_data->architecture->length; ++j)
    {
        tp =
            change_architecture_times_find
            (
                cp,
                cstate_data->architecture->list[j]
            );
        if (!tp->build_time || tp->build_time < t)
            wl.push_back_unique(tp->variant);
    }
    return outstanding_commentary(wl, cstate_data->architecture->length);
}


const char *
change_outstanding_tests(change::pointer cp, time_t t)
{
    cstate_ty       *cstate_data;
    cstate_architecture_times_ty *tp;
    size_t          j;

    assert(cp->reference_count >= 1);
    cstate_data = cp->cstate_get();
    string_list_ty wl;
    for (j = 0; j < cstate_data->architecture->length; ++j)
    {
        tp =
            change_architecture_times_find
            (
                cp,
                cstate_data->architecture->list[j]
            );
        if
        (
            !tp->test_time
        ||
            tp->test_time < t
        ||
            !tp->build_time
        ||
            tp->build_time < t
        )
            wl.push_back_unique(tp->variant);
    }
    return outstanding_commentary(wl, cstate_data->architecture->length);
}


const char *
change_outstanding_tests_baseline(change::pointer cp, time_t t)
{
    cstate_ty       *cstate_data;
    cstate_architecture_times_ty *tp;
    size_t          j;

    assert(cp->reference_count >= 1);
    cstate_data = cp->cstate_get();
    string_list_ty wl;
    for (j = 0; j < cstate_data->architecture->length; ++j)
    {
        tp =
            change_architecture_times_find
            (
                cp,
                cstate_data->architecture->list[j]
            );
        if
        (
            !tp->test_baseline_time
        ||
            tp->test_baseline_time < t
        ||
            !tp->build_time
        ||
            tp->build_time < t
        )
            wl.push_back_unique(tp->variant);
    }
    return outstanding_commentary(wl, cstate_data->architecture->length);
}


const char *
change_outstanding_tests_regression(change::pointer cp, time_t t)
{
    cstate_ty       *cstate_data;
    cstate_architecture_times_ty *tp;
    size_t          j;

    assert(cp->reference_count >= 1);
    cstate_data = cp->cstate_get();
    string_list_ty wl;
    for (j = 0; j < cstate_data->architecture->length; ++j)
    {
        tp =
            change_architecture_times_find
            (
                cp,
                cstate_data->architecture->list[j]
            );
        if
        (
            !tp->regression_test_time
        ||
            tp->regression_test_time < t
        ||
            !tp->build_time
        ||
            tp->build_time < t
        )
            wl.push_back_unique(tp->variant);
    }
    return outstanding_commentary(wl, cstate_data->architecture->length);
}


// vim: set ts=8 sw=4 et :
