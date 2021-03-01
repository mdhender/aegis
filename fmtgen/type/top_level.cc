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

#include <fmtgen/type/top_level.h>
#include <fmtgen/type/vector.h>


type_top_level::~type_top_level()
{
}


type_top_level::type_top_level(generator *a_gen, const pointer &a_subtype) :
    type(a_gen, a_subtype->def_name(), true),
    subtype(a_subtype)
{
}


bool
type_top_level::has_a_mask()
    const
{
    return subtype->has_a_mask();
}


nstring
type_top_level::c_name_inner()
    const
{
    return subtype->c_name();
}


void
type_top_level::get_reachable(type_vector &results)
    const
{
    results.push_back(subtype);
}
