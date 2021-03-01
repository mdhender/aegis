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

#include <common/error.h>
#include <common/trace.h>

#include <fmtgen/generator/introsp_code.h>
#include <fmtgen/type/boolean/introsp_code.h>
#include <fmtgen/type/enumeration/introsp_code.h>
#include <fmtgen/type/integer/introsp_code.h>
#include <fmtgen/type/list/introsp_code.h>
#include <fmtgen/type/real/introsp_code.h>
#include <fmtgen/type/string/introsp_code.h>
#include <fmtgen/type/structure/introsp_code.h>
#include <fmtgen/type/time/introsp_code.h>
#include <fmtgen/type/top_level/introsp_code.h>


generator_introspector_code::~generator_introspector_code()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


generator_introspector_code::generator_introspector_code(
        const nstring &filename) :
    generator(filename)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring incl_name = calculate_include_file_name(get_file_name());
    include_once(incl_name);
}


generator_introspector_code::pointer
generator_introspector_code::create(const nstring &filename)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new generator_introspector_code(filename));
}


void
generator_introspector_code::generate_file()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("generator_introspector_code::generate_file()\n"));
    // Nothing to do.
}


type::pointer
generator_introspector_code::type_boolean_factory()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_boolean_introspector_code::create(this);
}


type::pointer
generator_introspector_code::type_enum_factory(nstring const &name, bool global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_enumeration_introspector_code::create(this, name, global);
}


type::pointer
generator_introspector_code::type_integer_factory()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_integer_introspector_code::create(this);
}


type::pointer
generator_introspector_code::type_list_factory(const nstring &name, bool global,
    const type::pointer &subtype)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_list_introspector_code::create(this, name, global, subtype);
}


type::pointer
generator_introspector_code::type_real_factory()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_real_introspector_code::create(this);
}


type::pointer
generator_introspector_code::type_string_factory()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_string_introspector_code::create(this);
}


type::pointer
generator_introspector_code::type_structure_factory(const nstring &name,
    bool global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_structure_introspector_code::create(this, name, global);
}


type::pointer
generator_introspector_code::type_time_factory()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_time_introspector_code::create(this);
}


type::pointer
generator_introspector_code::top_level_factory(const type::pointer &subtype)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_top_level_introspector_code::create(this, subtype);
}
