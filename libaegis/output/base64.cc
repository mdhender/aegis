//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2004 Peter Miller;
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
// MANIFEST: functions to encode output as MIME base 64
//

#include <output/base64.h>
#include <output/private.h>
#include <str.h>

static char base64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#ifdef __GNUC__
static __inline int map64(int n) { return base64[n & 0x3F]; }
#else
#define map64(n) (base64[(n) & 0x3F])
#endif


typedef struct output_base64_ty output_base64_ty;
struct output_base64_ty
{
    output_ty	    inherited;
    output_ty	    *deeper;
    int		    delete_on_close;
    unsigned int    residual_value;
    int		    residual_bits;
    int		    output_column;
    long	    pos;
    int		    bol;
};


static void
output_base64_destructor(output_ty *fp)
{
    output_base64_ty *this_thing;

    this_thing = (output_base64_ty *)fp;
    switch (this_thing->residual_bits)
    {
    case 4:
	output_fputc(this_thing->deeper,
                     map64(this_thing->residual_value << 2));
	output_fputc(this_thing->deeper, '=');
	this_thing->output_column += 2;
	break;

    case 2:
	output_fputc(this_thing->deeper,
                     map64(this_thing->residual_value << 4));
	output_fputc(this_thing->deeper, '=');
	output_fputc(this_thing->deeper, '=');
	this_thing->output_column += 3;
	break;
    }
    if (this_thing->output_column)
	output_fputc(this_thing->deeper, '\n');
    if (this_thing->delete_on_close)
	output_delete(this_thing->deeper);
}


static void
output_base64_write(output_ty *fp, const void *p, size_t len)
{
    const unsigned char *data;
    output_base64_ty *this_thing;

    data = (unsigned char *)p;
    this_thing = (output_base64_ty *)fp;
    while (len > 0)
    {
	unsigned c = *data++;
	--len;

	this_thing->residual_value =
            (this_thing->residual_value << 8) | (c & 0xFF);
	this_thing->residual_bits += 8;
	for (;;)
	{
	    this_thing->residual_bits -= 6;
	    output_fputc
	    (
		this_thing->deeper,
		map64(this_thing->residual_value >> this_thing->residual_bits)
	    );
	    this_thing->output_column++;
	    if (this_thing->residual_bits == 0 &&
                this_thing->output_column > 70)
	    {
		output_fputc(this_thing->deeper, '\n');
		this_thing->output_column = 0;
	    }
	    if (this_thing->residual_bits < 6)
		break;
	}
	this_thing->pos++;
	this_thing->bol = (c == '\n');
    }
}


static string_ty *
output_base64_filename(output_ty *fp)
{
    output_base64_ty *this_thing;

    this_thing = (output_base64_ty *)fp;
    return output_filename(this_thing->deeper);
}


static long
output_base64_ftell(output_ty *fp)
{
    output_base64_ty *this_thing;

    this_thing = (output_base64_ty *)fp;
    return this_thing->pos;
}


static int
output_base64_page_width(output_ty *fp)
{
    output_base64_ty *this_thing;

    this_thing = (output_base64_ty *)fp;
    return output_page_width(this_thing->deeper);
}


static int
output_base64_page_length(output_ty *fp)
{
    output_base64_ty *this_thing;

    this_thing = (output_base64_ty *)fp;
    return output_page_length(this_thing->deeper);
}


static void
output_base64_eoln(output_ty *fp)
{
    output_base64_ty *this_thing;

    this_thing = (output_base64_ty *)fp;
    if (!this_thing->bol)
	output_fputc(fp, '\n');
}


static output_vtbl_ty vtbl =
{
    sizeof(output_base64_ty),
    output_base64_destructor,
    output_base64_filename,
    output_base64_ftell,
    output_base64_write,
    output_generic_flush,
    output_base64_page_width,
    output_base64_page_length,
    output_base64_eoln,
    "base64",
};


output_ty *
output_base64(output_ty *deeper, int delete_on_close)
{
    output_ty	    *result;
    output_base64_ty *this_thing;

    result = output_new(&vtbl);
    this_thing = (output_base64_ty *)result;
    this_thing->deeper = deeper;
    this_thing->delete_on_close = !!delete_on_close;
    this_thing->residual_value = 0;
    this_thing->residual_bits = 0;
    this_thing->output_column = 0;
    this_thing->pos = 0;
    this_thing->bol = 1;
    return result;
}
