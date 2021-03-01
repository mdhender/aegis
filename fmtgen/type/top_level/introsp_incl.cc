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

#include <fmtgen/type/top_level/introsp_incl.h>


type_top_level_introspector_include::~type_top_level_introspector_include()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type_top_level_introspector_include::type_top_level_introspector_include(
        generator *a_gen, const pointer &a_subtype) :
    type_top_level(a_gen, a_subtype)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type_top_level_introspector_include::pointer
type_top_level_introspector_include::create(generator *a_gen,
    const pointer &a_subtype)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new type_top_level_introspector_include(a_gen, a_subtype));
}


void
type_top_level_introspector_include::gen_body()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}
