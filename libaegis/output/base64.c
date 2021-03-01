/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002 Peter Miller;
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
 * MANIFEST: functions to encode output as MIME base 64
 */

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
    output_base64_ty *this;

    this = (output_base64_ty *)fp;
    switch (this->residual_bits)
    {
    case 4:
	output_fputc(this->deeper, map64(this->residual_value << 2));
	output_fputc(this->deeper, '=');
	this->output_column += 2;
	break;

    case 2:
	output_fputc(this->deeper, map64(this->residual_value << 4));
	output_fputc(this->deeper, '=');
	output_fputc(this->deeper, '=');
	this->output_column += 3;
	break;
    }
    if (this->output_column)
	output_fputc(this->deeper, '\n');
    if (this->delete_on_close)
	output_delete(this->deeper);
}


static void
output_base64_write(output_ty *fp, const void *p, size_t len)
{
    const unsigned char *data;
    output_base64_ty *this;

    data = p;
    this = (output_base64_ty *)fp;
    while (len > 0)
    {
	unsigned c = *data++;
	--len;

	this->residual_value = (this->residual_value << 8) | (c & 0xFF);
	this->residual_bits += 8;
	for (;;)
	{
	    this->residual_bits -= 6;
	    output_fputc
	    (
		this->deeper,
		map64(this->residual_value >> this->residual_bits)
	    );
	    this->output_column++;
	    if (this->residual_bits == 0 && this->output_column > 70)
	    {
		output_fputc(this->deeper, '\n');
		this->output_column = 0;
	    }
	    if (this->residual_bits < 6)
		break;
	}
	this->pos++;
	this->bol = (c == '\n');
    }
}


static string_ty *
output_base64_filename(output_ty *fp)
{
    output_base64_ty *this;

    this = (output_base64_ty *)fp;
    return output_filename(this->deeper);
}


static long
output_base64_ftell(output_ty *fp)
{
    output_base64_ty *this;

    this = (output_base64_ty *)fp;
    return this->pos;
}


static int
output_base64_page_width(output_ty *fp)
{
    output_base64_ty *this;

    this = (output_base64_ty *)fp;
    return output_page_width(this->deeper);
}


static int
output_base64_page_length(output_ty *fp)
{
    output_base64_ty *this;

    this = (output_base64_ty *)fp;
    return output_page_length(this->deeper);
}


static void
output_base64_eoln(output_ty *fp)
{
    output_base64_ty *this;

    this = (output_base64_ty *)fp;
    if (!this->bol)
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
    output_base64_ty *this;

    result = output_new(&vtbl);
    this = (output_base64_ty *)result;
    this->deeper = deeper;
    this->delete_on_close = !!delete_on_close;
    this->residual_value = 0;
    this->residual_bits = 0;
    this->output_column = 0;
    this->pos = 0;
    this->bol = 1;
    return result;
}
