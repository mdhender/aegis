/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate memorys
 */

#include <ac/string.h>

#include <error.h>
#include <mem.h>
#include <output/memory.h>
#include <output/private.h>
#include <str.h>
#include <trace.h>


typedef struct output_memory_ty output_memory_ty;
struct output_memory_ty
{
    output_ty       inherited;
    unsigned char   *buffer;
    size_t          size;
    size_t          maximum;
};


static void output_memory_destructor _((output_ty *));

static void
output_memory_destructor(fp)
    output_ty       *fp;
{
    output_memory_ty *this;

    trace(("output_memory_destructor(fp = %08lX)\n{\n", (long)fp));
    this = (output_memory_ty *)fp;
    if (this->buffer)
	mem_free(this->buffer);
    trace(("}\n"));
}


static string_ty *output_memory_filename _((output_ty *));

static string_ty *
output_memory_filename(fp)
    output_ty       *fp;
{
    static string_ty *s;

    if (!s)
	s = str_from_c("memory");
    return s;
}


static long output_memory_ftell _((output_ty *));

static long
output_memory_ftell(fp)
    output_ty       *fp;
{
    output_memory_ty *this;

    this = (output_memory_ty *)fp;
    return this->size;
}


static void output_memory_write _((output_ty *, const void *, size_t));

static void
output_memory_write(fp, data, len)
    output_ty       *fp;
    const void      *data;
    size_t          len;
{
    output_memory_ty *this;

    trace(("output_memory_write(fp = %08lX, data = %08lX, len = %ld)\n{\n",
	(long)fp, (long)data, (long)len));
    this = (output_memory_ty *)fp;
    while (this->size + len > this->maximum)
    {
	this->maximum = 2 * this->maximum + 32;
	this->buffer = mem_change_size(this->buffer, this->maximum);
    }
    memcpy(this->buffer + this->size, data, len);
    this->size += len;
    trace(("}\n"));
}


static void output_memory_eoln _((output_ty *));

static void
output_memory_eoln(fp)
    output_ty       *fp;
{
    output_memory_ty *this;

    trace(("output_memory_eol(fp = %08lX)\n{\n", (long)fp));
    this = (output_memory_ty *)fp;
    if (this->size && this->buffer[this->size - 1] != '\n')
	output_fputc(fp, '\n');
    trace(("}\n"));
}


static output_vtbl_ty vtbl =
{
    sizeof(output_memory_ty),
    output_memory_destructor,
    output_memory_filename,
    output_memory_ftell,
    output_memory_write,
    output_generic_flush,
    output_generic_page_width,
    output_generic_page_length,
    output_memory_eoln,
    "memory",
};


output_ty *
output_memory_open()
{
    output_ty       *result;
    output_memory_ty *this;

    trace(("output_memory_open()\n{\n"));
    result = output_new(&vtbl);
    this = (output_memory_ty *)result;
    this->buffer = 0;
    this->size = 0;
    this->maximum = 0;
    trace(("return %08lX\n", (long)this));
    trace(("}\n"));
    return result;
}


void
output_memory_forward(fp, deeper)
    output_ty       *fp;
    output_ty       *deeper;
{
    output_memory_ty *this;

    trace(("output_memory_forward(fp = %08lX, deeper = %08lX)\n{\n",
	(long)fp, (long)deeper));
    output_flush(fp);
    assert(fp->vptr == &vtbl);
    if (fp->vptr == &vtbl)
    {
	this = (output_memory_ty *)fp;
	if (this->size)
	    output_write(deeper, this->buffer, this->size);
#ifdef DEBUG
	else
	    error_raw("%s: %d: nothing to forward", __FILE__, __LINE__);
#endif
    }
    trace(("}\n"));
}
