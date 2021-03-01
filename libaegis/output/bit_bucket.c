/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate bit_buckets
 */

#include <page.h>
#include <output/bit_bucket.h>
#include <output/private.h>
#include <str.h>

typedef struct output_bit_bucket_ty output_bit_bucket_ty;
struct output_bit_bucket_ty
{
    output_ty       inherited;
    string_ty       *filename;
    size_t          pos;
};


static void
output_bit_bucket_destructor(output_ty *fp)
{
    output_bit_bucket_ty *this_thing;

    this_thing = (output_bit_bucket_ty *)fp;
    if (this_thing->filename)
	str_free(this_thing->filename);
    this_thing->filename = 0;
    this_thing->pos = 0;
}


static string_ty *
output_bit_bucket_filename(output_ty *fp)
{
    output_bit_bucket_ty *this_thing;

    this_thing = (output_bit_bucket_ty *)fp;
    if (!this_thing->filename)
	this_thing->filename = str_from_c("/dev/null");
    return this_thing->filename;
}


static long
output_bit_bucket_ftell(output_ty *fp)
{
    output_bit_bucket_ty *this_thing;

    this_thing = (output_bit_bucket_ty *)fp;
    return this_thing->pos;
}


static void
output_bit_bucket_write(output_ty *fp, const void *data, size_t len)
{
    output_bit_bucket_ty *this_thing;

    this_thing = (output_bit_bucket_ty *)fp;
    this_thing->pos += len;
}


static int
output_bit_bucket_page_width(output_ty *fp)
{
    return page_width_get(DEFAULT_PRINTER_WIDTH);
}


static int
output_bit_bucket_page_length(output_ty *fp)
{
    return page_length_get(DEFAULT_PRINTER_LENGTH);
}


static void
output_bit_bucket_eoln(output_ty *fp)
{
}



static output_vtbl_ty vtbl =
{
    sizeof(output_bit_bucket_ty),
    output_bit_bucket_destructor,
    output_bit_bucket_filename,
    output_bit_bucket_ftell,
    output_bit_bucket_write,
    output_generic_flush,
    output_bit_bucket_page_width,
    output_bit_bucket_page_length,
    output_bit_bucket_eoln,
    "file",
};


output_ty *
output_bit_bucket(void)
{
    output_ty       *result;
    output_bit_bucket_ty *this_thing;

    result = output_new(&vtbl);
    this_thing = (output_bit_bucket_ty *) result;
    this_thing->filename = 0;
    this_thing->pos = 0;
    return result;
}
