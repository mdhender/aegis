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
 * MANIFEST: functions to manipulate prefixs
 */

#include <output/prefix.h>
#include <output/private.h>
#include <str.h>

typedef struct output_prefix_ty output_prefix_ty;
struct output_prefix_ty
{
	output_ty	inherited;
	output_ty	*deeper;
	int		delete_on_close;
	string_ty	*prefix;
	int		column;
	long		pos;
};


static void output_prefix_destructor _((output_ty *));

static void
output_prefix_destructor(fp)
	output_ty	*fp;
{
	output_prefix_ty *this;

	this = (output_prefix_ty *)fp;
	if (this->delete_on_close)
		output_delete(this->deeper);
	str_free(this->prefix);
	this->prefix = 0;
	this->deeper = 0;
}


static void output_prefix_write _((output_ty *, const void *, size_t));

static void
output_prefix_write(fp, p, len)
	output_ty	*fp;
	const void	*p;
	size_t		len;
{
	const unsigned char *data;
	const unsigned char *begin;
	output_prefix_ty *this;
	int		c;
	size_t		nbytes;

	data = p;
	this = (output_prefix_ty *)fp;
	begin = data;
	while (len > 0)
	{
		c = *data++;
		--len;

		if (this->column == 0)
		{
			output_put_str(this->deeper, this->prefix);
			this->pos += this->prefix->str_length;
		}
		if (c == '\n')
		{
			nbytes = data - begin;
			output_write(this->deeper, begin, nbytes);
			this->column = 0;
			this->pos += nbytes;
			begin = data;
		}
		else
			this->column++;
	}
	if (data > begin)
	{
		nbytes = data - begin;
		output_write(this->deeper, begin, nbytes);
		this->pos += nbytes;
	}
}


static string_ty *output_prefix_filename _((output_ty *));

static string_ty *
output_prefix_filename(fp)
	output_ty	*fp;
{
	output_prefix_ty *this;

	this = (output_prefix_ty *)fp;
	return output_filename(this->deeper);
}


static long output_prefix_ftell _((output_ty *));

static long
output_prefix_ftell(fp)
	output_ty	*fp;
{
	output_prefix_ty *this;

	this = (output_prefix_ty *)fp;
	return this->pos;
}


static int output_prefix_page_width _((output_ty *));

static int
output_prefix_page_width(fp)
	output_ty	*fp;
{
	output_prefix_ty *this;

	this = (output_prefix_ty *)fp;
	return output_page_width(this->deeper);
}


static int output_prefix_page_length _((output_ty *));

static int
output_prefix_page_length(fp)
	output_ty	*fp;
{
	output_prefix_ty *this;

	this = (output_prefix_ty *)fp;
	return output_page_length(this->deeper);
}


static void output_prefix_eoln _((output_ty *));

static void
output_prefix_eoln(fp)
	output_ty	*fp;
{
	output_prefix_ty *this;

	this = (output_prefix_ty *)fp;
	if (this->column != 0)
		output_fputc(fp, '\n');
}


static output_vtbl_ty vtbl =
{
	sizeof(output_prefix_ty),
	output_prefix_destructor,
	output_prefix_filename,
	output_prefix_ftell,
	output_prefix_write,
	output_generic_flush,
	output_prefix_page_width,
	output_prefix_page_length,
	output_prefix_eoln,
	"prefix",
};


output_ty *
output_prefix(deeper, delete_on_close, prfx)
	output_ty	*deeper;
	int		delete_on_close;
	const char	*prfx;
{
	output_ty	*result;
	output_prefix_ty *this;

	result = output_new(&vtbl);
	this = (output_prefix_ty *)result;
	this->deeper = deeper;
	this->delete_on_close = !!delete_on_close;
	this->prefix = str_from_c(prfx);
	this->column = 0;
	this->pos = 0;
	return result;
}
