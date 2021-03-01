//
// aegis - project change supervisor
// Copyright (C) 2008, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/aer/expr/name.h>
#include <libaegis/aer/value/enum.h>
#include <libaegis/introspector/enumeration.h>
#include <libaegis/sub.h>


introspector_enumeration::~introspector_enumeration()
{
}


introspector_enumeration::introspector_enumeration(const nstring &a_name,
        const adapter::pointer &a_how) :
    name(a_name),
    how(a_how)
{
    members.set_reaper();
}


void
introspector_enumeration::register_tags(const char **names, size_t len)
{
    for (size_t j = 0; j < len; ++j)
        members.assign(names[j], int(j));
}


nstring
introspector_enumeration::get_name()
    const
{
    return name;
}


void
introspector_enumeration::enumeration(const nstring &member)
{
    trace(("%s {\n", __PRETTY_FUNCTION__));
    trace(("member = %s;\n", member.quote_c().c_str()));

    int *value_p = members.query(member);
    if (value_p)
    {
        int value = *value_p;
        how->set(value);
        trace(("}\n"));
        return;
    }

    nstring guess = members.query_fuzzy(member);
    if (guess.empty())
    {
        value_of_type_required();
        trace(("}\n"));
        return;
    }

    sub_context_ty sc;
    sc.var_set_string("Name", member);
    sc.var_set_string("Guess", guess);
    error(&sc, i18n("no \"$name\", guessing \"$guess\""));

    value_p = members.query(guess);
    assert(value_p);
    int value = *value_p;
    how->set(value);
    trace(("}\n"));
}


void
introspector_enumeration::report_init(const char **names, size_t len)
{
    for (size_t j = 0; j < len; ++j)
    {
        nstring name(names[j]);
        rpt_value::pointer value = rpt_value_enumeration::create(j, name);
        rpt_expr_name__init(name, value);
    }
}


// vim: set ts=8 sw=4 et :
