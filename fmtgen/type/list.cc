//
// aegis - project change supervisor
// Copyright (C) 1991-1994, 1998, 1999, 2001-2008, 2012 Peter Miller
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

#include <fmtgen/indent.h>
#include <fmtgen/type/list.h>
#include <fmtgen/type/vector.h>


type_list::~type_list()
{
}


type_list::type_list(generator *a_gen, const nstring &a_name, bool a_global,
        const type::pointer &a_type) :
    type(a_gen, a_name, a_global),
    subtype(a_type)
{
}


void
type_list::in_include_file()
{
    assert(subtype);
    type::in_include_file();
    subtype->in_include_file();
}


nstring
type_list::c_name_inner()
    const
{
    return (def_name() + "_ty *");
}


bool
type_list::has_a_mask()
    const
{
    return false;
}


void
type_list::get_reachable(type_vector &results)
    const
{
    results.push_back(subtype);
}


// vim: set ts=8 sw=4 et :
