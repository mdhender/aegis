//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate crops
//

#include <input/crop.h>
#include <trace.h>


input_crop::~input_crop()
{
    trace(("~input_crop()\n{\n"));
    if (pos < maximum)
	deeper->skip(maximum - pos);
    if (delete_on_close)
	delete deeper;
    deeper = 0;
    trace(("}\n"));
}


input_crop::input_crop(input_ty *arg1, bool arg2, long arg3) :
    deeper(arg1),
    delete_on_close(arg2),
    maximum(arg3),
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
