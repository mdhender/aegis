//
//	aegis - project change supervisor
//	Copyright (C) 2001-2004 Peter Miller;
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
// MANIFEST: functions to manipulate uuencodes
//

#include <ac/string.h>

#include <output/private.h>
#include <output/uuencode.h>
#include <str.h>

static char etab[] =
           "`!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";

typedef struct output_uuencode_ty output_uuencode_ty;
struct output_uuencode_ty
{
    output_ty	    inherited;
    output_ty	    *deeper;
    int		    delete_on_close;
    unsigned int    residual_value;
    int		    residual_bits;
    char	    obuf[64 * 4 / 3];
    int		    opos;
    int		    ipos;
    long	    pos;
    int		    bol;
    int		    begun;
    string_ty	    *filename;
};


static void
output_uuencode_destructor(output_ty *fp)
{
    output_uuencode_ty *this_thing;

    this_thing = (output_uuencode_ty *)fp;
    if (this_thing->begun)
    {
	while (this_thing->residual_bits > 0)
	{
	    int		    n;

	    if (this_thing->residual_bits < 6)
	    {
		// fake a NUL character
		this_thing->residual_value <<= 8;
		this_thing->residual_bits += 8;
	    }

	    this_thing->residual_bits -= 6;
	    n = (this_thing->residual_value >> this_thing->residual_bits);
	    n &= 0x3F;
	    this_thing->obuf[this_thing->opos++] = etab[n];
	}
	if (this_thing->ipos)
	{
	    this_thing->obuf[0] = etab[this_thing->ipos];
	    this_thing->obuf[this_thing->opos++] = '\n';
	    output_write(this_thing->deeper,
                         this_thing->obuf,
                         this_thing->opos);
	    this_thing->ipos = 0;
	    this_thing->opos = 1; // The length character
	}
	output_fputs(this_thing->deeper, "`\nend\n");
    }
    if (this_thing->delete_on_close)
	output_delete(this_thing->deeper);
    if (this_thing->filename)
	str_free(this_thing->filename);
    this_thing->delete_on_close = 0;
    this_thing->deeper = 0;
    this_thing->filename = 0;
}


static void
output_uuencode_write(output_ty *fp, const void *p, size_t len)
{
    const unsigned char *data;
    output_uuencode_ty *this_thing;

    this_thing = (output_uuencode_ty *)fp;
    if (!this_thing->begun)
    {
	this_thing->begun = 1;
	output_fputs(this_thing->deeper, "begin 644 ");
	output_put_str(this_thing->deeper, output_filename(fp));
	output_fputc(this_thing->deeper, '\n');
    }
    data = (unsigned char *)p;
    if (len > 0)
	this_thing->bol = (data[len - 1] == '\n');
    this_thing->pos += len;
    while (len > 0)
    {
	unsigned char   c;

	c = *data++;
	--len;

	this_thing->residual_value =
            (this_thing->residual_value << 8) | (c & 0xFF);
	this_thing->ipos++;
	this_thing->residual_bits += 8;
	for (;;)
	{
	    int		    n;

	    this_thing->residual_bits -= 6;
	    n = (this_thing->residual_value >> this_thing->residual_bits);
	    n &= 0x3F;
	    this_thing->obuf[this_thing->opos++] = etab[n];
	    if (this_thing->residual_bits == 0 && this_thing->opos > 60)
	    {
		this_thing->obuf[0] = etab[this_thing->ipos];
		this_thing->obuf[this_thing->opos++] = '\n';
		output_write(this_thing->deeper,
                             this_thing->obuf,
                             this_thing->opos);
		this_thing->opos = 1; // The length character
		this_thing->ipos = 0;
	    }
	    if (this_thing->residual_bits < 6)
		break;
	}
    }
}


static string_ty *
output_uuencode_filename(output_ty *fp)
{
    output_uuencode_ty *this_thing;

    this_thing = (output_uuencode_ty *)fp;
    if (!this_thing->filename)
    {
	char		*cp1;
	char		*cp2;
	char		*cp3;

	this_thing->filename = output_filename(this_thing->deeper);
	cp1 = this_thing->filename->str_text;
	cp2 = strrchr(cp1, '/');
	if (cp2)
	    ++cp2;
	else
	    cp2 = cp1;
	cp3 = strstr(cp2, ".uu");
	if (cp3)
	    this_thing->filename = str_n_from_c(cp1, cp3 - cp1);
	else
	    this_thing->filename = str_copy(this_thing->filename);
    }
    return this_thing->filename;
}


static long
output_uuencode_ftell(output_ty *fp)
{
    output_uuencode_ty *this_thing;

    this_thing = (output_uuencode_ty *)fp;
    return this_thing->pos;
}


static int
output_uuencode_page_width(output_ty *fp)
{
    return 80;
}


static int
output_uuencode_page_length(output_ty *fp)
{
    return 66;
}


static void
output_uuencode_eoln(output_ty *fp)
{
    output_uuencode_ty *this_thing;

    this_thing = (output_uuencode_ty *)fp;
    if (!this_thing->bol)
	output_fputc(fp, '\n');
}


static output_vtbl_ty vtbl =
{
    sizeof(output_uuencode_ty),
    output_uuencode_destructor,
    output_uuencode_filename,
    output_uuencode_ftell,
    output_uuencode_write,
    output_generic_flush,
    output_uuencode_page_width,
    output_uuencode_page_length,
    output_uuencode_eoln,
    "uuencode",
};


output_ty *
output_uuencode(output_ty *deeper, int delete_on_close)
{
    output_ty	    *result;
    output_uuencode_ty *this_thing;

    result = output_new(&vtbl);
    this_thing = (output_uuencode_ty *)result;
    this_thing->deeper = deeper;
    this_thing->delete_on_close = !!delete_on_close;
    this_thing->residual_value = 0;
    this_thing->residual_bits = 0;
    this_thing->opos = 1; // The length character
    this_thing->ipos = 0;
    this_thing->pos = 0;
    this_thing->bol = 1;
    this_thing->begun = 0;
    this_thing->filename = 0;
    return result;
}
