//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate catenates
//

#include <error.h>
#include <input/catenate.h>
#include <mem.h>
#include <str.h>
#include <trace.h>


input_catenate::~input_catenate()
{
    trace(("input_catenate::~input_catenate()\n{\n"));
    pullback_transfer(deeper[selector]);
    if (delete_on_close)
    {
	for (size_t j = 0; j < ndeeper; ++j)
	{
	    delete deeper[j];
	    deeper[j] = 0;
	}
    }
    delete deeper;
    deeper = 0;
    trace(("}\n"));
}


input_catenate::input_catenate(input_ty **arg1, size_t arg2, bool arg3) :
    deeper(new input_ty * [arg2]),
    ndeeper(arg2),
    delete_on_close(arg3),
    selector(0),
    pos(0)
{
    trace(("input_catenate(arg1 = %08lX, arg2 = %ld)\n{\n", (long)arg1,
	(long)arg2));
    for (size_t j = 0; j < arg2; ++j)
	deeper[j] = arg1[j];
    trace(("}\n"));
}


long
input_catenate::read_inner(void *data, size_t len)
{
    size_t nbytes = 0;
    trace(("input_catenate::read_inner()\n{\n"));
    for (;;)
    {
	assert(selector < ndeeper);
	input_ty *fp = deeper[selector];
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


long
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
    input_ty *fp = deeper[selector];
    return fp->name();
}


long
input_catenate::length()
{
    long result = 0;
    for (size_t j = 0; j < ndeeper; ++j)
    {
	long len = deeper[j]->length();
	if (len < 0)
	{
	    result = -1;
	    break;
	}
	result += len;
    }
    trace(("input_catenate::length => %ld\n", result));
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
    input_ty *fp = deeper[selector];
    return fp->is_remote();
}
