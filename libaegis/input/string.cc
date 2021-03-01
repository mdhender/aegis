//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>

#include <libaegis/input/string.h>


input_string::~input_string()
{
}


input_string::input_string(const nstring &arg) :
    base(arg),
    pos(0)
{
}


long
input_string::read_inner(void *data, size_t len)
{
    if (pos >= base.size())
	return 0;
    size_t nbytes = base.size() - pos;
    if (nbytes > len)
	nbytes = len;
    memcpy(data, base.c_str() + pos, nbytes);
    pos += nbytes;
    return nbytes;
}


long
input_string::ftell_inner()
{
    return pos;
}


nstring
input_string::name()
{
    return "generated string";
}


long
input_string::length()
{
    return base.size();
}
