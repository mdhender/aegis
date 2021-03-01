//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
#include <input/private.h>
#include <mem.h>
#include <str.h>
#include <trace.h>


typedef struct input_catenate_ty input_catenate_ty;
struct input_catenate_ty
{
    input_ty        inherited;
    input_ty        **deeper;
    size_t          ndeeper;
    int             delete_on_close;
    size_t          selector;
    size_t          pos;
};


static void
input_catenate_destructor(input_ty *p)
{
    input_catenate_ty *this_thing;
    size_t          j;

    trace(("input_catenate_destructor()\n{\n"));
    this_thing = (input_catenate_ty *)p;
    input_pushback_transfer(this_thing->deeper[this_thing->selector], p);
    if (this_thing->delete_on_close)
    {
	for (j = 0; j < this_thing->ndeeper; ++j)
	{
	    input_delete(this_thing->deeper[j]);
	    this_thing->deeper[j] = 0;
	}
    }
    this_thing->deeper = 0;
    trace(("}\n"));
}


static long
input_catenate_read(input_ty *ip, void *data, size_t len)
{
    input_catenate_ty *this_thing;
    size_t          nbytes;
    input_ty        *fp;

    trace(("input_catenate_read()\n{\n"));
    this_thing = (input_catenate_ty *)ip;
    for (;;)
    {
	assert(this_thing->selector < this_thing->ndeeper);
	fp = this_thing->deeper[this_thing->selector];
	nbytes = input_read(fp, data, len);
	if (nbytes)
	    break;
	if (this_thing->selector + 1 >= this_thing->ndeeper)
	    break;
	this_thing->selector++;
    }
    this_thing->pos += nbytes;
    trace(("return %ld;\n", (long)nbytes));
    trace(("}\n"));
    return nbytes;
}


static long
input_catenate_ftell(input_ty *fp)
{
    input_catenate_ty *this_thing;

    this_thing = (input_catenate_ty *)fp;
    trace(("input_catenate_ftell => %ld\n", (long)this_thing->pos));
    return this_thing->pos;
}


static string_ty *
input_catenate_name(input_ty *p)
{
    input_catenate_ty *this_thing;
    input_ty        *fp;

    trace(("input_catenate_name\n"));
    this_thing = (input_catenate_ty *)p;
    assert(this_thing->selector < this_thing->ndeeper);
    fp = this_thing->deeper[this_thing->selector];
    return input_name(fp);
}


static long
input_catenate_length(input_ty *p)
{
    input_catenate_ty *this_thing;
    long            result;
    size_t          j;
    long            len;

    this_thing = (input_catenate_ty *)p;
    result = 0;
    for (j = 0; j < this_thing->ndeeper; ++j)
    {
	len = input_length(this_thing->deeper[j]);
	if (len < 0)
	{
	    result = -1;
	    break;
	}
	result += len;
    }
    trace(("input_catenate_length => %ld\n", result));
    return result;
}


static void
input_catenate_keepalive(input_ty *fp)
{
    input_catenate_ty *ip;
    size_t          j;

    ip = (input_catenate_ty *)fp;
    for (j = 0; j < ip->ndeeper; ++j)
	input_keepalive(ip->deeper[j]);
}


static input_vtbl_ty vtbl =
{
    sizeof(input_catenate_ty),
    input_catenate_destructor,
    input_catenate_read,
    input_catenate_ftell,
    input_catenate_name,
    input_catenate_length,
    input_catenate_keepalive,
};


input_ty *
input_catenate(input_ty **fpl, size_t nfp, int del)
{
    input_ty        *result;
    input_catenate_ty *this_thing;
    size_t          j;

    trace(("input_catenate(fpl = %08lX, nfp = %ld)\n{\n",
	    (long)fpl, (long)nfp));
    result = input_new(&vtbl);
    this_thing = (input_catenate_ty *)result;
    this_thing->ndeeper = nfp;
    this_thing->deeper = (input_ty **)mem_alloc(nfp * sizeof(input_ty *));
    for (j = 0; j < nfp; ++j)
	this_thing->deeper[j] = fpl[j];
    this_thing->delete_on_close = !!del;
    this_thing->selector = 0;
    this_thing->pos = 0;
    trace(("return %08lX\n", (long)result));
    trace(("}\n"));
    return result;
}
