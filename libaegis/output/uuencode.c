/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate uuencodes
 */

#include <ac/string.h>

#include <output/private.h>
#include <output/uuencode.h>
#include <str.h>

static char etab[] =
	"`!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";

typedef struct output_uuencode_ty output_uuencode_ty;
struct output_uuencode_ty
{
	output_ty	inherited;
	output_ty	*deeper;
	int		delete_on_close;
	unsigned int	residual_value;
	int		residual_bits;
	char		obuf[64*4/3];
	int		opos;
	int		ipos;
	long		pos;
	int		bol;
	int		begun;
	string_ty	*filename;
};


static void output_uuencode_destructor _((output_ty *));

static void
output_uuencode_destructor(fp)
	output_ty	*fp;
{
	output_uuencode_ty *this;

	this = (output_uuencode_ty *)fp;
	if (this->begun)
	{
		while (this->residual_bits > 0)
		{
			int		n;
	
			if (this->residual_bits < 6)
			{
				/* fake a NUL character */
				this->residual_value <<= 8;
				this->residual_bits += 8;
			}

			this->residual_bits -= 6;
			n = (this->residual_value >> this->residual_bits);
			n &= 0x3F;
			this->obuf[this->opos++] = etab[n];
		}
		if (this->ipos)
		{
			this->obuf[0] = etab[this->ipos];
			this->obuf[this->opos++] = '\n';
			output_write(this->deeper, this->obuf, this->opos);
			this->ipos = 0;
			this->opos = 1;
		}
		output_fputs(this->deeper, "`\nend\n");
	}
	if (this->delete_on_close)
		output_delete(this->deeper);
	if (this->filename)
		str_free(this->filename);
	this->delete_on_close = 0;
	this->deeper = 0;
	this->filename = 0;
}


static void output_uuencode_write _((output_ty *, const void *, size_t));

static void
output_uuencode_write(fp, p, len)
	output_ty	*fp;
	const void	*p;
	size_t		len;
{
	const unsigned char *data;
	output_uuencode_ty *this;

	this = (output_uuencode_ty *)fp;
	if (!this->begun)
	{
		this->begun = 1;
		output_fputs(this->deeper, "begin 644 ");
		output_put_str(this->deeper, output_filename(fp));
		output_fputc(this->deeper, '\n');
	}
	data = p;
	if (len > 0)
		this->bol = (data[len - 1] == '\n');
	this->pos += len;
	while (len > 0)
	{
		unsigned c = *data++;
		--len;

		this->residual_value = (this->residual_value << 8) | (c & 0xFF);
		this->ipos++;
		this->residual_bits += 8;
		for (;;)
		{
			int		n;

			this->residual_bits -= 6;
			n = (this->residual_value >> this->residual_bits);
			n &= 0x3F;
			this->obuf[this->opos++] = etab[n];
			if (this->residual_bits == 0 && this->opos > 60)
			{
				this->obuf[0] = etab[this->ipos]; 
				this->obuf[this->opos++] = '\n';
				output_write(this->deeper, this->obuf, this->opos);
				this->opos = 1;
				this->ipos = 0;
			}
			if (this->residual_bits < 6)
				break;
		}
	}
}


static string_ty *output_uuencode_filename _((output_ty *));

static string_ty *
output_uuencode_filename(fp)
	output_ty	*fp;
{
	output_uuencode_ty *this;

	this = (output_uuencode_ty *)fp;
	if (!this->filename)
	{
		char	*cp1;
		char	*cp2;
		char	*cp3;

		this->filename = output_filename(this->deeper);
		cp1 = this->filename->str_text;
		cp2 = strrchr(cp1, '/');
		if (cp2)
			++cp2;
		else
			cp2 = cp1;
		cp3 = strstr(cp2, ".uu");
		if (cp3)
			this->filename = str_n_from_c(cp1, cp3 - cp1);
		else
			this->filename = str_copy(this->filename);
	}
	return this->filename;
}


static long output_uuencode_ftell _((output_ty *));

static long
output_uuencode_ftell(fp)
	output_ty	*fp;
{
	output_uuencode_ty *this;

	this = (output_uuencode_ty *)fp;
	return this->pos;
}


static int output_uuencode_page_width _((output_ty *));

static int
output_uuencode_page_width(fp)
	output_ty	*fp;
{
	return 80;
}


static int output_uuencode_page_length _((output_ty *));

static int
output_uuencode_page_length(fp)
	output_ty	*fp;
{
	return 66;
}


static void output_uuencode_eoln _((output_ty *));

static void
output_uuencode_eoln(fp)
	output_ty	*fp;
{
	output_uuencode_ty *this;

	this = (output_uuencode_ty *)fp;
	if (!this->bol)
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
output_uuencode(deeper, delete_on_close)
	output_ty	*deeper;
	int		delete_on_close;
{
	output_ty	*result;
	output_uuencode_ty *this;

	result = output_new(&vtbl);
	this = (output_uuencode_ty *)result;
	this->deeper = deeper;
	this->delete_on_close = !!delete_on_close;
	this->residual_value = 0;
	this->residual_bits = 0;
	this->opos = 1;
	this->ipos = 0;
	this->pos = 0;
	this->bol = 1;
	this->filename = 0;
	return result;
}
