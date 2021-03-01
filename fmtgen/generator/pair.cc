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

#include <fmtgen/generator/pair.h>
#include <fmtgen/type/pair.h>


generator_pair::~generator_pair()
{
}


generator_pair::generator_pair(const pointer &a_g1, const pointer &a_g2) :
    generator("/dev/null"),
    g1(a_g1),
    g2(a_g2)
{
}


generator_pair::pointer
generator_pair::create(const pointer &a_g1, const pointer &a_g2)
{
    return pointer(new generator_pair(a_g1, a_g2));
}


type::pointer
generator_pair::type_boolean_factory()
{
    type::pointer t1 = g1->type_boolean_factory();
    type::pointer t2 = g2->type_boolean_factory();
    return type_pair::create(this, t1, t2);
}


type::pointer
generator_pair::type_enum_factory(const nstring &name, bool global)
{
    return
        type_pair::create
        (
            this,
            g1->type_enum_factory(name, global),
            g2->type_enum_factory(name, global)
        );
}


type::pointer
generator_pair::type_integer_factory()
{
    return
        type_pair::create
        (
            this,
            g1->type_integer_factory(),
            g2->type_integer_factory()
        );
}


type::pointer
generator_pair::type_list_factory(const nstring &name, bool global,
    const type::pointer &subtype)
{
    type_pair *pp = dynamic_cast<type_pair *>(subtype.get());
    assert(pp);
    return
        type_pair::create
        (
            this,
            g1->type_list_factory(name, global, pp->get_t1()),
            g2->type_list_factory(name, global, pp->get_t2())
        );
}


type::pointer
generator_pair::type_real_factory()
{
    return
        type_pair::create
        (
            this,
            g1->type_real_factory(),
            g2->type_real_factory()
        );
}


type::pointer
generator_pair::type_string_factory()
{
    return
        type_pair::create
        (
            this,
            g1->type_string_factory(),
            g2->type_string_factory()
        );
}


type::pointer
generator_pair::type_structure_factory(const nstring &name, bool global)
{
    return
        type_pair::create
        (
            this,
            g1->type_structure_factory(name, global),
            g2->type_structure_factory(name, global)
        );
}


type::pointer
generator_pair::type_time_factory()
{
    return
        type_pair::create
        (
            this,
            g1->type_time_factory(),
            g2->type_time_factory()
        );
}


void
generator_pair::generate_file()
{
    g1->generate_file();
    g2->generate_file();
}


type::pointer
generator_pair::top_level_factory(const type::pointer &subtype)
{
    type_pair *pp = dynamic_cast<type_pair *>(subtype.get());
    assert(pp);
    return
        type_pair::create
        (
            this,
            g1->top_level_factory(pp->get_t1()),
            g2->top_level_factory(pp->get_t2())
        );
}


// vim: set ts=8 sw=4 et :
