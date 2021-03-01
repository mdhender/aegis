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

#include <fmtgen/generator/orig_code.h>
#include <fmtgen/type/boolean/code.h>
#include <fmtgen/type/enumeration/code.h>
#include <fmtgen/type/integer/code.h>
#include <fmtgen/type/list/code.h>
#include <fmtgen/type/real/code.h>
#include <fmtgen/type/string/code.h>
#include <fmtgen/type/structure/code.h>
#include <fmtgen/type/time/code.h>
#include <fmtgen/type/top_level/code.h>


generator_original_code::~generator_original_code()
{
}


generator_original_code::generator_original_code(const nstring &filename) :
    generator(filename)
{
    nstring incl_name = calculate_include_file_name(get_file_name());
    include_once(incl_name);
}


generator_original_code::pointer
generator_original_code::create(const nstring &filename)
{
    return pointer(new generator_original_code(filename));
}


void
generator_original_code::generate_file()
{
    trace(("generator_original_code::generate_file()\n"));
    // Nothing to do.
}


type::pointer
generator_original_code::type_boolean_factory()
{
    return type_boolean_code::create(this);
}


type::pointer
generator_original_code::type_enum_factory(nstring const &name, bool global)
{
    return type_enumeration_code::create(this, name, global);
}


type::pointer
generator_original_code::type_integer_factory()
{
    return type_integer_code::create(this);
}


type::pointer
generator_original_code::type_list_factory(const nstring &name, bool global,
    const type::pointer &subtype)
{
    return type_list_code::create(this, name, global, subtype);
}


type::pointer
generator_original_code::type_real_factory()
{
    return type_real_code::create(this);
}


type::pointer
generator_original_code::type_string_factory()
{
    return type_string_code::create(this);
}


type::pointer
generator_original_code::type_structure_factory(const nstring &name,
    bool global)
{
    return type_structure_code::create(this, name, global);
}


type::pointer
generator_original_code::type_time_factory()
{
    return type_time_code::create(this);
}


type::pointer
generator_original_code::top_level_factory(const type::pointer &subtype)
{
    return type_top_level_code::create(this, subtype);
}
