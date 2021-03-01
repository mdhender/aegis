/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002, 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate outstandings
 */

#include <change.h>
#include <error.h> /* for assert */
#include <str_list.h>
#include <sub.h>


static char *
outstanding_commentary(string_list_ty *wlp, long narch)
{
    static string_ty *s;

    if (s)
    {
	str_free(s);
	s = 0;
    }
    if (!wlp->nstrings || wlp->nstrings >= narch)
	s = str_from_c("");
    else
    {
	sub_context_ty	*scp;
	string_ty	*t1;
	string_ty	*t2;
	size_t		j;

	t1 = str_format("\"%S\"", wlp->string[0]);
	for (j = 1; j < wlp->nstrings; ++j)
	{
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name1", t1);
	    sub_var_set_format(scp, "Name2", "\"%S\"", wlp->string[j]);
	    if (j == wlp->nstrings - 1)
		t2 = subst_intl(scp, i18n("$name1 and $name2"));
	    else
		t2 = subst_intl(scp, i18n("$name1, $name2"));
	    sub_context_delete(scp);
	    str_free(t1);
	    t1 = t2;
	}
	scp = sub_context_new();
	sub_var_set_string(scp, "Name_List", t1);
	sub_var_set_long(scp, "Number", (long)wlp->nstrings);
	sub_var_optional(scp, "Number");
	s = subst_intl(scp, i18n("for the $name_list architectures"));
	sub_context_delete(scp);
	str_free(t1);
    }
    string_list_destructor(wlp);
    return s->str_text;
}


const char *
change_outstanding_builds(change_ty *cp, time_t t)
{
    cstate	    cstate_data;
    string_list_ty  wl;
    cstate_architecture_times tp;
    long	    j;

    assert(cp->reference_count >= 1);
    cstate_data = change_cstate_get(cp);
    string_list_constructor(&wl);
    for (j = 0; j < cstate_data->architecture->length; ++j)
    {
	tp =
	    change_architecture_times_find
	    (
	       	cp,
	       	cstate_data->architecture->list[j]
	    );
	if (!tp->build_time || tp->build_time < t)
	    string_list_append_unique(&wl, tp->variant);
    }
    return outstanding_commentary(&wl, cstate_data->architecture->length);
}


const char *
change_outstanding_tests(change_ty *cp, time_t t)
{
    cstate	    cstate_data;
    string_list_ty  wl;
    cstate_architecture_times tp;
    long	    j;

    assert(cp->reference_count >= 1);
    cstate_data = change_cstate_get(cp);
    string_list_constructor(&wl);
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
	    string_list_append_unique(&wl, tp->variant);
    }
    return outstanding_commentary(&wl, cstate_data->architecture->length);
}


const char *
change_outstanding_tests_baseline(change_ty *cp, time_t t)
{
    cstate	    cstate_data;
    string_list_ty  wl;
    cstate_architecture_times tp;
    long	    j;

    assert(cp->reference_count >= 1);
    cstate_data = change_cstate_get(cp);
    string_list_constructor(&wl);
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
	    string_list_append_unique(&wl, tp->variant);
    }
    return outstanding_commentary(&wl, cstate_data->architecture->length);
}


const char *
change_outstanding_tests_regression(change_ty *cp, time_t t)
{
    cstate	    cstate_data;
    string_list_ty  wl;
    cstate_architecture_times tp;
    long	    j;

    assert(cp->reference_count >= 1);
    cstate_data = change_cstate_get(cp);
    string_list_constructor(&wl);
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
	    string_list_append_unique(&wl, tp->variant);
    }
    return outstanding_commentary(&wl, cstate_data->architecture->length);
}
