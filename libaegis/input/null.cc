//
//      aegis - project change supervisor
//      Copyright (C) 2006-2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <libaegis/input/null.h>


input_null::~input_null()
{
}


input_null::input_null()
{
}

nstring
input_null::name()
{
    return "/dev/null";
}

off_t
input_null::length()
{
    return 0;
}

ssize_t
input_null::read_inner(void *, size_t)
{
    return 0;
}

off_t
input_null::ftell_inner()
{
    return 0;
}


// vim: set ts=8 sw=4 et :
