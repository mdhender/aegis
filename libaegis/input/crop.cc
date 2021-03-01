//
//      aegis - project change supervisor
//      Copyright (C) 2004-2006, 2008, 2012 Peter Miller
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

#include <libaegis/input/crop.h>
#include <common/trace.h>


input_crop::~input_crop()
{
    trace(("~input_crop()\n{\n"));
    if (pos < maximum)
        deeper->skip(maximum - pos);
    trace(("}\n"));
}


input_crop::input_crop(input &arg1, size_t arg2) :
    deeper(arg1),
    maximum(arg2),
    pos(0)
{
    trace(("input_crop()\n"));
}


ssize_t
input_crop::read_inner(void *data, size_t len)
{
    trace(("input_crop::read_inner()\n{\n"));

    if (pos + len > maximum)
    {
        //
        // The cast below is safe since (maximum - pos) < len < SIZE_MAX.
        //
        len = (size_t)(maximum - pos);
    }

    ssize_t nbytes = deeper->read(data, len);
    if (nbytes > 0)
        pos += nbytes;
    trace(("return %zd\n", nbytes));
    trace(("}\n"));
    return nbytes;
}


off_t
input_crop::ftell_inner()
{
    trace(("input_crop::ftell_inner => %ld\n", (long)pos));
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


off_t
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


// vim: set ts=8 sw=4 et :
