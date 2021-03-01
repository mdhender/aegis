//
// aegis - project change supervisor
// Copyright (C) 2001-2009, 2011, 2012 Peter Miller
// Copyright (C) 2008, 2009 Walter Franzini
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
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
#include <common/ac/ctype.h>
#include <common/ac/time.h>

#include <common/language.h>
#include <common/nstring.h>
#include <common/sizeof.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/change/attributes.h>
#include <libaegis/change/branch.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/sub.h>
#include <libaegis/sub/change/number.h>


#define ONE_OR_MORE (-1)


typedef nstring (*func_ptr)(change::pointer , const wstring_list &);
struct table_ty
{
    const char      *name;
    func_ptr        func;
    int             num_args;
};


static nstring
change_number_get(change::pointer cp, const wstring_list &)
{
    return nstring::format("%ld", magic_zero_decode(cp->number));
}


static nstring
change_attribute_get(change::pointer cp, const wstring_list &arg)
{
    assert(arg.size() >= 3);
    nstring name = arg[2].to_nstring();
    nstring value(change_attributes_find(cp, name.get_ref()));
    return value;
}


static nstring
get_brief_description(change::pointer cp, const wstring_list &)
{
    //
    // We don't actually return the whole brief description, just the
    // first line, or the first 80 characters, whichever is shortest.
    //
    cstate_ty *cstate_data = cp->cstate_get();
    nstring s(cstate_data->brief_description);
    size_t len = 0;
    for (; len < s.size() && len < 80; ++len)
        if (s[len] == '\n')
            break;
    return s.substr(0, len);
}


static nstring
get_delta(change::pointer cp, const wstring_list &)
{
    cstate_ty *cstate_data = cp->cstate_get();
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
        return "";

    case cstate_state_being_integrated:
    case cstate_state_completed:
        break;
    }
    return nstring::format("%ld", cstate_data->delta_number);
}


static nstring
get_delta_uuid(change::pointer cp, const wstring_list &)
{
    cstate_ty *cstate_data = cp->cstate_get();
    switch (cstate_data->state)
    {
    case cstate_state_being_integrated:
    case cstate_state_completed:
        return nstring(cstate_data->delta_uuid);

    case cstate_state_awaiting_development:
    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
        break;
    }
    return "";
}


static nstring
get_development_directory(change::pointer cp, const wstring_list &)
{
    cstate_ty *cstate_data = cp->cstate_get();
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_completed:
        return "";

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
    case cstate_state_being_integrated:
        break;
    }
    return nstring(change_development_directory_get(cp, 0));
}


static nstring
get_developer(change::pointer cp, const wstring_list &)
{
    cstate_ty *cstate_data = cp->cstate_get();
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
        return "";

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
    case cstate_state_being_integrated:
    case cstate_state_completed:
        break;
    }
    return nstring(cp->developer_name());
}


static nstring
get_integration_directory(change::pointer cp, const wstring_list &)
{
    cstate_ty *cstate_data = cp->cstate_get();
    if (cstate_data->state != cstate_state_being_integrated)
        return "";
    return nstring(change_integration_directory_get(cp, 0));
}


static nstring
calc_date_string(time_t when, const wstring_list &arg)
{
    nstring fmt = arg.unsplit(2, arg.size()).to_nstring();
    struct tm *the_time = localtime(&when);

    //
    // The strftime is locale dependent.
    //
    language_human();
    char buf[1000];
    strftime(buf, sizeof(buf), fmt.c_str(), the_time);
    language_C();

    return nstring(buf);
}


static nstring
get_integrate_pass_date(change::pointer cp, const wstring_list &arg)
{
    cstate_ty *cstate_data = cp->cstate_get();
    if (cstate_data->state != cstate_state_completed)
        return "";
    time_t when = cp->completion_timestamp();
    return calc_date_string(when, arg);
}


static nstring
get_integrator(change::pointer cp, const wstring_list &)
{
    cstate_ty *cstate_data = cp->cstate_get();
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
        return "";

    case cstate_state_being_integrated:
    case cstate_state_completed:
        break;
    }
    return nstring(cp->integrator_name());
}


static nstring
get_reviewer(change::pointer cp, const wstring_list &)
{
    cstate_ty       *cstate_data;

    cstate_data = cp->cstate_get();
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
        return "";

    case cstate_state_awaiting_integration:
    case cstate_state_being_integrated:
    case cstate_state_completed:
        break;
    }
    return nstring(cp->reviewer_name());
}


static nstring
get_state(change::pointer cp, const wstring_list &)
{
    cstate_ty *cstate_data = cp->cstate_get();
    return nstring(cstate_state_ename(cstate_data->state));
}


static nstring
get_cause(change::pointer cp, const wstring_list &)
{
    cstate_ty *cstate_data = cp->cstate_get();
    return nstring(change_cause_ename(cstate_data->cause));
}


static nstring
get_uuid(change::pointer cp, const wstring_list &)
{
    cstate_ty *cstate_data = cp->cstate_get();
    return nstring(cstate_data->uuid);
}


static nstring
get_version(change::pointer cp, const wstring_list &)
{
    return cp->version_get();
}


static nstring
get_debian_version(change::pointer cp, const wstring_list &)
{
    return cp->version_debian_get();
}


static nstring
get_rpm_version(change::pointer cp, const wstring_list &)
{
    return cp->version_rpm_get();
}


static table_ty table[] =
{
    { "attribute", change_attribute_get, 1 },
    { "brief_description", get_brief_description, 0 },
    { "cause", get_cause, 0 },
    { "completion_date", get_integrate_pass_date, ONE_OR_MORE },
    { "date", get_integrate_pass_date, ONE_OR_MORE },
    { "debian-version", get_debian_version, 0 },
    { "delta", get_delta, 0 },
    { "delta_uuid", get_delta_uuid, 0 },
    { "description", get_brief_description, 0 },
    { "developer", get_developer, 0 },
    { "development_directory", get_development_directory, 0 },
    { "integrate_pass_date", get_integrate_pass_date, ONE_OR_MORE },
    { "integration_directory", get_integration_directory, 0 },
    { "integrator", get_integrator, 0 },
    { "number", change_number_get, 0 },
    { "reviewer", get_reviewer, 0 },
    { "rpm-version", get_rpm_version, 0 },
    { "state", get_state, 0 },
    { "uuid", get_uuid, 0 },
    { "version", get_version, 0 },
};


static symtab_ty *stp;


static table_ty *
find_func(const nstring &name)
{
    if (!stp)
    {
        stp = new symtab_ty(SIZEOF(table));
        for (table_ty *tp = table; tp < ENDOF(table); ++tp)
        {
            nstring s(tp->name);
            stp->assign(s, tp);
        }
    }
    table_ty *result = (table_ty *)stp->query(name);
    if (!result)
    {
        nstring s(stp->query_fuzzy(name.downcase()));
        if (!s.empty())
        {
            sub_context_ty sc;
            sc.var_set_string("Name", name);
            sc.var_set_string("Guess", s);
            sc.error_intl(i18n("no \"$name\", guessing \"$guess\""));

            result = (table_ty *)stp->query(s);
            assert(result);
        }
    }
    return result;
}


static const char *
requires_at_least_n_arguments(int n)
{
    switch (n)
    {
    case 1:
        return i18n("requires at least one argument");

    case 2:
        return i18n("requires two or more arguments");
    }
    return i18n("invalid function arguments");
}


static const char *
requires_exactly_n_arguments(int n)
{
    switch (n)
    {
    case 0:
        return i18n("requires zero arguments");

    case 1:
        return i18n("requires one argument");

    case 2:
        return i18n("requires two arguments");

    case 3:
        return i18n("requires three arguments");
    }
    return i18n("invalid function arguments");
}


//
// NAME
//      sub_change - the change substitution
//
// SYNOPSIS
//      wstring_ty *sub_change(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_change function implements the change substitution.
//      The change substitution is replaced by the change number.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_change_number(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_change()\n{\n"));
    change::pointer cp = sub_context_change_get(scp);
    wstring result;
    if (!cp || cp->bogus)
    {
        scp->error_set(i18n("not valid in current context"));
        trace(("}\n"));
        return result;
    }
    if (arg.size() <= 1)
    {
        nstring s(change_number_get(cp, arg));
        result = wstring(s);
        trace(("return %p;\n", result.get_ref()));
        trace(("}\n"));
        return result;
    }

    nstring s = arg[1].to_nstring();
    table_ty *tp = find_func(s);
    if (!tp)
    {
        scp->error_set(i18n("unknown substitution variant"));
        trace(("}\n"));
        return result;
    }

    int num_args = tp->num_args;
    bool at_least = false;
    if (num_args < 0)
    {
        num_args = -num_args;
        at_least = true;
    }

    if (at_least && (arg.size() < (size_t)num_args + 2))
    {
        scp->error_set(requires_at_least_n_arguments(num_args));
        trace(("}\n"));
        return result;
    }
    else if (!at_least && (arg.size() != (size_t)num_args + 2))
    {
        scp->error_set(requires_exactly_n_arguments(num_args));
        trace(("}\n"));
        return result;
    }

    s = tp->func(cp, arg);
    if (s.empty())
    {
        scp->error_set(i18n("not valid in current context"));
        trace(("}\n"));
        return result;
    }

    result = wstring(s);
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
