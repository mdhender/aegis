//
//      aegis - project change supervisor
//      Copyright (C) 2002-2006, 2008, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <common/mem.h>
#include <common/str.h>
#include <common/trace.h>
#include <libaegis/input/catenate.h>


input_catenate::~input_catenate()
{
    trace(("input_catenate::~input_catenate()\n{\n"));
    pullback_transfer(deeper[selector]);
    delete [] deeper;
    deeper = 0;
    trace(("}\n"));
}


input_catenate::input_catenate(input arg1[], size_t arg2, bool arg3) :
    deeper(new input[arg2]),
    ndeeper(arg2),
    delete_on_close(arg3),
    selector(0),
    pos(0)
{
    trace(("input_catenate(arg1 = %p, arg2 = %ld)\n{\n", arg1,
           (long)arg2));
    for (size_t j = 0; j < arg2; ++j)
        deeper[j] = arg1[j];
    trace(("}\n"));
}


ssize_t
input_catenate::read_inner(void *data, size_t len)
{
    size_t nbytes = 0;
    trace(("input_catenate::read_inner()\n{\n"));
    for (;;)
    {
        assert(selector < ndeeper);
        input &fp = deeper[selector];
        nbytes = fp->read(data, len);
        if (nbytes)
            break;
        if (selector + 1 >= ndeeper)
            break;
        selector++;
    }
    pos += nbytes;
    trace(("return %ld;\n", (long)nbytes));
    trace(("}\n"));
    return nbytes;
}


off_t
input_catenate::ftell_inner()
{
    trace(("input_catenate::ftell_inner => %ld\n", (long)pos));
    return pos;
}


nstring
input_catenate::name()
{
    trace(("input_catenate::name\n"));
    assert(selector < ndeeper);
    input &fp = deeper[selector];
    return fp->name();
}


off_t
input_catenate::length()
{
    off_t result = 0;
    for (size_t j = 0; j < ndeeper; ++j)
    {
        off_t len = deeper[j]->length();
        if (len < 0)
        {
            result = -1;
            break;
        }
        result += len;
    }
    // fixme: the cast in the following statement should not be there.
    trace(("input_catenate::length => %ld\n", (long)result));
    return result;
}


void
input_catenate::keepalive()
{
    for (size_t j = 0; j < ndeeper; ++j)
        deeper[j]->keepalive();
}


bool
input_catenate::is_remote()
    const
{
    trace(("input_catenate::is_remote\n"));
    assert(selector < ndeeper);
    input &fp = deeper[selector];
    return fp->is_remote();
}


// vim: set ts=8 sw=4 et :
