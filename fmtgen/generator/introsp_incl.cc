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

#include <fmtgen/generator/introsp_incl.h>
#include <fmtgen/type/boolean/introsp_incl.h>
#include <fmtgen/type/enumeration/introsp_incl.h>
#include <fmtgen/type/integer/introsp_incl.h>
#include <fmtgen/type/list/introsp_incl.h>
#include <fmtgen/type/real/introsp_incl.h>
#include <fmtgen/type/string/introsp_incl.h>
#include <fmtgen/type/structure/introsp_incl.h>
#include <fmtgen/type/time/introsp_incl.h>
#include <fmtgen/type/top_level/introsp_incl.h>


generator_introspector_include::~generator_introspector_include()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


generator_introspector_include::generator_introspector_include(
        const nstring &filnam) :
    generator(filnam)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf("\n");
    nstring insulator = get_file_name().upcase().identifier();
    printf("#ifndef %s\n", insulator.c_str());
    printf("#define %s\n", insulator.c_str());
}


generator::pointer
generator_introspector_include::create(const nstring &filnam)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new generator_introspector_include(filnam));
}


void
generator_introspector_include::generate_file()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf("\n");
    nstring insulator = get_file_name().upcase().identifier();
    printf("#endif // %s\n", insulator.c_str());
}


type::pointer
generator_introspector_include::type_boolean_factory()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_boolean_introspector_include::create(this);
}


type::pointer
generator_introspector_include::type_enum_factory(nstring const &name,
    bool global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_enumeration_introspector_include::create(this, name, global);
}


type::pointer
generator_introspector_include::type_integer_factory()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_integer_introspector_include::create(this);
}


type::pointer
generator_introspector_include::type_list_factory(const nstring &name,
    bool global, const type::pointer &subtype)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_list_introspector_include::create(this, name, global, subtype);
}


type::pointer
generator_introspector_include::type_real_factory()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_real_introspector_include::create(this);
}


type::pointer
generator_introspector_include::type_string_factory()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_string_introspector_include::create(this);
}


type::pointer
generator_introspector_include::type_structure_factory(const nstring &name,
    bool global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_structure_introspector_include::create(this, name, global);
}


type::pointer
generator_introspector_include::type_time_factory()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_time_introspector_include::create(this);
}


type::pointer
generator_introspector_include::top_level_factory(const type::pointer &subtype)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return type_top_level_introspector_include::create(this, subtype);
}
