//
// aegis - project change supervisor
// Copyright (C) 1991-1994, 1997-1999, 2001-2008, 2012 Peter Miller
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

#include <fmtgen/indent.h>
#include <fmtgen/parse.h>
#include <fmtgen/type/enumeration.h>


type_enumeration::~type_enumeration()
{
}


type_enumeration::type_enumeration(generator *a_gen, const nstring &a_name,
        bool a_global) :
    type(a_gen, a_name, a_global)
{
}


void
type_enumeration::member_add(const nstring &member_name, const type::pointer &,
    int, const nstring &)
{
    elements.push_back(member_name);
}


nstring
type_enumeration::c_name_inner()
    const
{
    return nstring::format("%s_ty", def_name().c_str());
}


bool
type_enumeration::has_a_mask()
    const
{
    return true;
}


// vim: set ts=8 sw=4 et :
