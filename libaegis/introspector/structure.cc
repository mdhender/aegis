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

#include <libaegis/introspector/error.h>
#include <libaegis/introspector/structure.h>
#include <libaegis/sub.h>


introspector_structure::~introspector_structure()
{
}


introspector_structure::introspector_structure(const nstring &a_name) :
    name(a_name)
{
    members.set_reaper();
}


introspector_structure::pointer
introspector_structure::create(const nstring &a_name)
{
    return pointer(new introspector_structure(a_name));
}


nstring
introspector_structure::get_name()
    const
{
    return name;
}


introspector::pointer
introspector_structure::field(const nstring &field_name)
{
    nstring fname = field_name;
    adapter::pointer p = members.get(fname);
    if (!p)
    {
        nstring suggest = members.query_fuzzy(fname);
        if (suggest.empty())
        {
            sub_context_ty sc;
            sc.var_set_string("Name", field_name);
            error(&sc, i18n("the name \"$name\" is undefined"));
            return introspector_error::create();
        }

        sub_context_ty sc;
        sc.var_set_string("Name", name);
        sc.var_set_string("Guess", suggest);
        error(&sc, i18n("no \"$name\", guessing \"$guess\""));

        fname = suggest;
        p = members.get(fname);
        assert(p);
    }

    if (p->is_set() && !p->redefinition_ok())
    {
        sub_context_ty sc;
        sc.var_set_string("Name", field_name);
        error(&sc, i18n("field \"$name\" redefined"));
    }

    return p->introspector_factory();
}


// vim: set ts=8 sw=4 et :
