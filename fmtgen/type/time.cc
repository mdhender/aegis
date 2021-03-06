//
// aegis - project change supervisor
// Copyright (C) 1993, 1994, 1998, 1999, 2002-2008, 2012 Peter Miller
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
#include <fmtgen/type/time.h>


type_time::~type_time()
{
}


type_time::type_time(generator *a_gen) :
    type(a_gen, "time", true)
{
}


nstring
type_time::c_name_inner()
    const
{
    return "time_t";
}


bool
type_time::has_a_mask()
    const
{
    return true;
}


// vim: set ts=8 sw=4 et :
