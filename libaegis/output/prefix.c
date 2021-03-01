/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2003 Peter Miller;
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


static void
output_prefix_destructor(output_ty *fp)
{
	output_prefix_ty *this_thing;

	this_thing = (output_prefix_ty *)fp;
	if (this_thing->delete_on_close)
		output_delete(this_thing->deeper);
	str_free(this_thing->prefix);
	this_thing->prefix = 0;
	this_thing->deeper = 0;
}


static void
output_prefix_write(output_ty *fp, const void *p, size_t len)
{
	const unsigned char *data;
	const unsigned char *begin;
	output_prefix_ty *this_thing;
	int		c;
	size_t		nbytes;

	data = (unsigned char *)p;
	this_thing = (output_prefix_ty *)fp;
	begin = data;
	while (len > 0)
	{
		c = *data++;
		--len;

		if (this_thing->column == 0)
		{
			output_put_str(this_thing->deeper, this_thing->prefix);
			this_thing->pos += this_thing->prefix->str_length;
		}
		if (c == '\n')
		{
			nbytes = data - begin;
			output_write(this_thing->deeper, begin, nbytes);
			this_thing->column = 0;
			this_thing->pos += nbytes;
			begin = data;
		}
		else
			this_thing->column++;
	}
	if (data > begin)
	{
		nbytes = data - begin;
		output_write(this_thing->deeper, begin, nbytes);
		this_thing->pos += nbytes;
	}
}


static string_ty *
output_prefix_filename(output_ty *fp)
{
	output_prefix_ty *this_thing;

	this_thing = (output_prefix_ty *)fp;
	return output_filename(this_thing->deeper);
}


static long
output_prefix_ftell(output_ty *fp)
{
	output_prefix_ty *this_thing;

	this_thing = (output_prefix_ty *)fp;
	return this_thing->pos;
}


static int
output_prefix_page_width(output_ty *fp)
{
	output_prefix_ty *this_thing;

	this_thing = (output_prefix_ty *)fp;
	return output_page_width(this_thing->deeper);
}


static int
output_prefix_page_length(output_ty *fp)
{
	output_prefix_ty *this_thing;

	this_thing = (output_prefix_ty *)fp;
	return output_page_length(this_thing->deeper);
}


static void
output_prefix_eoln(output_ty *fp)
{
	output_prefix_ty *this_thing;

	this_thing = (output_prefix_ty *)fp;
	if (this_thing->column != 0)
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
output_prefix(output_ty *deeper, int delete_on_close, const char *prfx)
{
	output_ty	*result;
	output_prefix_ty *this_thing;

	result = output_new(&vtbl);
	this_thing = (output_prefix_ty *)result;
	this_thing->deeper = deeper;
	this_thing->delete_on_close = !!delete_on_close;
	this_thing->prefix = str_from_c(prfx);
	this_thing->column = 0;
	this_thing->pos = 0;
	return result;
}
