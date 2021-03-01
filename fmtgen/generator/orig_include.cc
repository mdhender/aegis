//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
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

#include <common/trace.h>

#include <fmtgen/generator/orig_include.h>
#include <fmtgen/type/boolean/include.h>
#include <fmtgen/type/enumeration/include.h>
#include <fmtgen/type/integer/include.h>
#include <fmtgen/type/list/include.h>
#include <fmtgen/type/real/include.h>
#include <fmtgen/type/string/include.h>
#include <fmtgen/type/structure/include.h>
#include <fmtgen/type/time/include.h>
#include <fmtgen/type/top_level/include.h>


generator_original_include::~generator_original_include()
{
}


generator_original_include::generator_original_include(const nstring &filnam) :
    generator(filnam)
{
    printf("\n");
    nstring insulator = get_file_name().upcase().identifier();
    printf("#ifndef %s\n", insulator.c_str());
    printf("#define %s\n", insulator.c_str());
}


generator::pointer
generator_original_include::create(const nstring &filnam)
{
    return pointer(new generator_original_include(filnam));
}


void
generator_original_include::generate_file()
{
    trace(("generator_original_include::generate_file()\n{\n"));
    printf("\n");
    nstring insulator = get_file_name().upcase().identifier();
    printf("#endif // %s\n", insulator.c_str());
    trace(("}\n"));
}


type::pointer
generator_original_include::type_boolean_factory()
{
    return type_boolean_include::create(this);
}


type::pointer
generator_original_include::type_enum_factory(nstring const &name, bool global)
{
    return type_enumeration_include::create(this, name, global);
}


type::pointer
generator_original_include::type_integer_factory()
{
    return type_integer_include::create(this);
}


type::pointer
generator_original_include::type_list_factory(const nstring &name, bool global,
    const type::pointer &subtype)
{
    return type_list_include::create(this, name, global, subtype);
}


type::pointer
generator_original_include::type_real_factory()
{
    return type_real_include::create(this);
}


type::pointer
generator_original_include::type_string_factory()
{
    return type_string_include::create(this);
}


type::pointer
generator_original_include::type_structure_factory(const nstring &name,
    bool global)
{
    return type_structure_include::create(this, name, global);
}


type::pointer
generator_original_include::type_time_factory()
{
    return type_time_include::create(this);
}


type::pointer
generator_original_include::top_level_factory(const type::pointer &subtype)
{
    return type_top_level_include::create(this, subtype);
}
