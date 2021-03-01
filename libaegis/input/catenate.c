/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate catenates
 */

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
    int             selector;
    size_t          pos;
};


static void input_catenate_destructor _((input_ty *));

static void
input_catenate_destructor(p)
    input_ty        *p;
{
    input_catenate_ty *this;
    size_t          j;

    trace(("input_catenate_destructor()\n{\n"));
    this = (input_catenate_ty *)p;
    input_pushback_transfer(this->deeper[this->selector], p);
    if (this->delete_on_close)
    {
	for (j = 0; j < this->ndeeper; ++j)
	{
	    input_delete(this->deeper[j]);
	    this->deeper[j] = 0;
	}
    }
    this->deeper = 0;
    trace(("}\n"));
}


static long input_catenate_read _((input_ty *, void *, size_t));

static long
input_catenate_read(ip, data, len)
    input_ty        *ip;
    void            *data;
    size_t          len;
{
    input_catenate_ty *this;
    size_t          nbytes;
    input_ty        *fp;

    trace(("input_catenate_read()\n{\n"));
    this = (input_catenate_ty *)ip;
    for (;;)
    {
	assert(this->selector < this->ndeeper);
	fp = this->deeper[this->selector];
	nbytes = input_read(fp, data, len);
	if (nbytes)
	    break;
	if (this->selector + 1 >= this->ndeeper)
	    break;
	this->selector++;
    }
    this->pos += nbytes;
    trace(("return %ld;\n", (long)nbytes));
    trace(("}\n"));
    return nbytes;
}


static long input_catenate_ftell _((input_ty *));

static long
input_catenate_ftell(fp)
    input_ty        *fp;
{
    input_catenate_ty *this;

    this = (input_catenate_ty *)fp;
    trace(("input_catenate_ftell => %ld\n", (long)this->pos));
    return this->pos;
}


static string_ty *input_catenate_name _((input_ty *));

static string_ty *
input_catenate_name(p)
    input_ty        *p;
{
    input_catenate_ty *this;
    input_ty        *fp;

    trace(("input_catenate_name\n"));
    this = (input_catenate_ty *)p;
    assert(this->selector < this->ndeeper);
    fp = this->deeper[this->selector];
    return input_name(fp);
}


static long input_catenate_length _((input_ty *));

static long
input_catenate_length(p)
    input_ty        *p;
{
    input_catenate_ty *this;
    long            result;
    size_t          j;
    long            len;

    this = (input_catenate_ty *)p;
    result = 0;
    for (j = 0; j < this->ndeeper; ++j)
    {
	len = input_length(this->deeper[j]);
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


static input_vtbl_ty vtbl =
{
    sizeof(input_catenate_ty),
    input_catenate_destructor,
    input_catenate_read,
    input_catenate_ftell,
    input_catenate_name,
    input_catenate_length,
};


input_ty *
input_catenate(fpl, nfp, del)
    input_ty        **fpl;
    size_t          nfp;
    int             del;
{
    input_ty        *result;
    input_catenate_ty *this;
    size_t          j;

    trace(("input_catenate(fpl = %08lX, nfp = %ld)\n{\n",
	    (long)fpl, (long)nfp));
    result = input_new(&vtbl);
    this = (input_catenate_ty *)result;
    this->ndeeper = nfp;
    this->deeper = mem_alloc(nfp * sizeof(input_ty *));
    for (j = 0; j < nfp; ++j)
	this->deeper[j] = fpl[j];
    this->delete_on_close = !!del;
    this->selector = 0;
    this->pos = 0;
    trace(("return %08lX\n", (long)result));
    trace(("}\n"));
    return result;
}
