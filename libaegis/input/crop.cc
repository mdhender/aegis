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

#include <libaegis/input/crop.h>
#include <common/trace.h>


input_crop::~input_crop()
{
    trace(("~input_crop()\n{\n"));
    if (pos < maximum)
	deeper->skip(maximum - pos);
    trace(("}\n"));
}


input_crop::input_crop(input &arg1, long arg2) :
    deeper(arg1),
    maximum(arg2),
    pos(0)
{
    trace(("input_crop()\n"));
}


long
input_crop::read_inner(void *data, size_t len)
{
    trace(("input_crop::read_inner()\n{\n"));
    if (pos + (long)len > maximum)
	len = maximum - pos;
    long nbytes = deeper->read(data, len);
    if (nbytes > 0)
	pos += nbytes;
    trace(("return %ld;\n", (long)nbytes));
    trace(("}\n"));
    return nbytes;
}


long
input_crop::ftell_inner()
{
    trace(("input_crop::ftell_inner => %ld\n", pos));
    return pos;
}


nstring
input_crop::name()
{
    trace(("input_crop::name()\n"));
    if (name_cache.empty())
	name_cache = deeper->name();
    return name_cache;
}


long
input_crop::length()
{
    return maximum;
}


void
input_crop::keepalive()
{
    deeper->keepalive();
}


bool
input_crop::is_remote()
    const
{
    return deeper->is_remote();
}
