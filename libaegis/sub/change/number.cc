//
//	aegis - project change supervisor
//	Copyright (C) 2001-2004 Peter Miller;
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
// MANIFEST: functions to manipulate changes
//

#include <ac/ctype.h>
#include <ac/time.h>

#include <change/branch.h>
#include <cstate.h>
#include <language.h>
#include <str.h>
#include <sub.h>
#include <sub/change/number.h>
#include <symtab.h>
#include <trace.h>
#include <wstr.h>
#include <wstr_list.h>


#define ONE_OR_MORE (-1)


typedef string_ty *(*func_ptr)(change_ty *, wstring_list_ty *);
struct table_ty
{
    const char      *name;
    func_ptr        func;
    int		    num_args;
};


static string_ty *
change_number_get(change_ty *cp, wstring_list_ty *arg)
{
    return str_format("%ld", magic_zero_decode(cp->number));
}


static string_ty *
change_description_get(change_ty *cp, wstring_list_ty *arg)
{
    cstate_ty       *cstate_data;
    string_ty	    *s;
    size_t	    len;

    //
    // We don't actually return the whole brief description, just the
    // first line, or the first 80 characters, whichever is shortest.
    //
    cstate_data = change_cstate_get(cp);
    s = cstate_data->brief_description;
    for (len = 0; len < s->str_length && len < 80; ++len)
	if (s->str_text[len] == '\n')
	    break;
    return str_n_from_c(s->str_text, len);
}


static string_ty *
get_delta(change_ty *cp, wstring_list_ty *arg)
{
    cstate_ty       *cstate_data;

    cstate_data = change_cstate_get(cp);
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
	return 0;

    case cstate_state_being_integrated:
    case cstate_state_completed:
	break;
    }
    return str_format("%ld", cstate_data->delta_number);
}


static string_ty *
get_development_directory(change_ty *cp, wstring_list_ty *arg)
{
    cstate_ty       *cstate_data;

    cstate_data = change_cstate_get(cp);
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_completed:
	return 0;

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
    case cstate_state_being_integrated:
	break;
    }
    return str_copy(change_development_directory_get(cp, 0));
}


static string_ty *
get_developer(change_ty *cp, wstring_list_ty *arg)
{
    cstate_ty       *cstate_data;

    cstate_data = change_cstate_get(cp);
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
	return 0;

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
    case cstate_state_being_integrated:
    case cstate_state_completed:
	break;
    }
    return str_copy(change_developer_name(cp));
}


static string_ty *
get_integration_directory(change_ty *cp, wstring_list_ty *arg)
{
    cstate_ty       *cstate_data;

    cstate_data = change_cstate_get(cp);
    if (cstate_data->state != cstate_state_being_integrated)
	return 0;
    return str_copy(change_integration_directory_get(cp, 0));
}


static string_ty *
calc_date_string(time_t when, wstring_list_ty *arg)
{
    struct tm	    *the_time;
    char	    buf[1000];
    size_t	    nbytes;
    wstring_ty	    *wfmt;
    string_ty	    *fmt;
    string_ty	    *result;

    wfmt = wstring_list_to_wstring(arg, 2, 32767, (char *)0);
    fmt = wstr_to_str(wfmt);
    wstr_free(wfmt);
    the_time = localtime(&when);

    //
    // The strftime is locale dependent.
    //
    language_human();
    nbytes = strftime(buf, sizeof(buf) - 1, fmt->str_text, the_time);
    language_C();

    result = str_n_from_c(buf, nbytes);
    str_free(fmt);
    return result;
}


static string_ty *
get_integrate_pass_date(change_ty *cp, wstring_list_ty *arg)
{
    cstate_ty       *cstate_data;
    time_t	    when;

    cstate_data = change_cstate_get(cp);
    if (cstate_data->state != cstate_state_completed)
	return 0;

    when = change_completion_timestamp(cp);
    return calc_date_string(when, arg);
}


static string_ty *
get_integrator(change_ty *cp, wstring_list_ty *arg)
{
    cstate_ty       *cstate_data;

    cstate_data = change_cstate_get(cp);
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
	return 0;

    case cstate_state_being_integrated:
    case cstate_state_completed:
	break;
    }
    return str_copy(change_integrator_name(cp));
}


static string_ty *
get_reviewer(change_ty *cp, wstring_list_ty *arg)
{
    cstate_ty       *cstate_data;

    cstate_data = change_cstate_get(cp);
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
	return 0;

    case cstate_state_awaiting_integration:
    case cstate_state_being_integrated:
    case cstate_state_completed:
	break;
    }
    return str_copy(change_reviewer_name(cp));
}


static string_ty *
get_state(change_ty *cp, wstring_list_ty *arg)
{
    cstate_ty       *cstate_data;

    cstate_data = change_cstate_get(cp);
    return str_from_c(cstate_state_ename(cstate_data->state));
}


static string_ty *
get_cause(change_ty *cp, wstring_list_ty *arg)
{
    cstate_ty       *cstate_data;

    cstate_data = change_cstate_get(cp);
    return str_from_c(change_cause_ename(cstate_data->cause));
}


static string_ty *
get_version(change_ty *cp, wstring_list_ty *arg)
{
    return str_copy(change_version_get(cp));
}


static table_ty table[] =
{
    {"brief_description", change_description_get, },
    {"cause", get_cause, },
    {"completion_date", get_integrate_pass_date, ONE_OR_MORE, },
    {"date", get_integrate_pass_date, ONE_OR_MORE, },
    {"delta", get_delta, },
    {"description", change_description_get, },
    {"developer", get_developer, },
    {"development_directory", get_development_directory, },
    {"integrate_pass_date", get_integrate_pass_date, ONE_OR_MORE, },
    {"integration_directory", get_integration_directory, },
    {"integrator", get_integrator, },
    {"number", change_number_get, },
    {"reviewer", get_reviewer, },
    {"state", get_state, },
    {"version", get_version, },
};


static symtab_ty *stp;


static table_ty *
find_func(string_ty *name)
{
    table_ty	    *tp;
    string_ty	    *s;
    table_ty	    *result;

    if (!stp)
    {
	stp = symtab_alloc(SIZEOF(table));
	for (tp = table; tp < ENDOF(table); ++tp)
	{
	    s = str_from_c(tp->name);
	    symtab_assign(stp, s, tp);
	    str_free(s);
	}
    }
    result = (table_ty *)symtab_query(stp, name);
    if (!result)
    {
	s = symtab_query_fuzzy(stp, name);
	if (s)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", name);
	    sub_var_set_string(scp, "Guess", s);
	    error_intl(scp, i18n("no \"$name\", guessing \"$guess\""));
	    sub_context_delete(scp);
	}
	return 0;
    }
    return result;
}


//
// NAME
//	sub_change - the change substitution
//
// SYNOPSIS
//	wstring_ty *sub_change(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_change function implements the change substitution.
//	The change substitution is replaced by the change number.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_change_number(sub_context_ty *scp, wstring_list_ty *arg)
{
    change_ty	    *cp;
    wstring_ty	    *result;
    string_ty	    *s;
    table_ty	    *tp;

    trace(("sub_change()\n{\n"));
    cp = sub_context_change_get(scp);
    result = 0;
    if (!cp || cp->bogus)
    {
	sub_context_error_set(scp, i18n("not valid in current context"));
	goto done;
    }
    if (arg->nitems <= 1)
    {
	s = change_number_get(cp, arg);
	result = str_to_wstr(s);
	str_free(s);
	goto done;
    }

    s = wstr_to_str(arg->item[1]);
    tp = find_func(s);
    str_free(s);
    if (!tp)
    {
	sub_context_error_set(scp, i18n("unknown substitution variant"));
	goto done;
    }

    if (tp->num_args == ONE_OR_MORE)
    {
	if (arg->nitems == 2)
	{
	    sub_context_error_set(scp, i18n("requires two arguments"));
	    goto done;
	}
    }
    else
    {
	if (arg->nitems != 2)
	{
	    sub_context_error_set(scp, i18n("requires one argument"));
	    goto done;
	}
    }

    s = tp->func(cp, arg);
    if (!s)
    {
	sub_context_error_set(scp, i18n("not valid in current context"));
	goto done;
    }

    result = str_to_wstr(s);
    str_free(s);

    done:
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
