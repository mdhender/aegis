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

#include <fmtgen/type/structure.h>
#include <fmtgen/type/vector.h>


type_structure::~type_structure()
{
    delete [] element;
    nelements = 0;
    nelements_max = 0;
    element = 0;
    toplevel_flag = false;
}


type_structure::type_structure(generator *a_gen, const nstring &a_name,
        bool a_global) :
    type(a_gen, a_name, a_global),
    nelements(0),
    nelements_max(0),
    element(0),
    toplevel_flag(false)
{
}


void
type_structure::member_add(const nstring &member_name,
    const type::pointer &member_type, int attributes, const nstring &cmnt)
{
    if (nelements >= nelements_max)
    {
        size_t new_nelements_max = nelements_max * 2 + 16;
        element_ty *new_element = new element_ty[new_nelements_max];
        for (size_t j = 0; j < nelements; ++j)
            new_element[j] = element[j];
        delete [] element;
        nelements_max = new_nelements_max;
        element = new_element;
    }
    element_ty *ep = element + nelements++;
    ep->etype = member_type;
    ep->name = member_name;
    ep->attributes = attributes;
    ep->comment = cmnt;
}


void
type_structure::in_include_file()
{
    type::in_include_file();
    for (size_t j = 0; j < nelements; ++j)
    {
        element[j].etype->in_include_file();
    }
}


void
type_structure::get_reachable(type_vector &results)
    const
{
    for (size_t j = 0; j < nelements; ++j)
        results.push_back(element[j].etype);
}


nstring
type_structure::c_name_inner()
    const
{
    return (def_name() + "_ty *");
}


bool
type_structure::has_a_mask()
    const
{
    return false;
}


void
type_structure::toplevel()
{
    type::toplevel();
    toplevel_flag = true;
}


// vim: set ts=8 sw=4 et :
