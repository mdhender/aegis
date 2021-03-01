/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2003 Peter Miller;
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
 * MANIFEST: functions to perfom rfc822 line wrapping
 */

#include <input/822wrap.h>
#include <input/private.h>


typedef struct input_822wrap_ty input_822wrap_ty;
struct input_822wrap_ty
{
	input_ty	inherit;
	input_ty	*deeper;
	int		close_on_delete;
	long		pos;
	int		column;
};


static void
input_822wrap_destructor(input_ty *fp)
{
	input_822wrap_ty *this_thing;

	this_thing = (input_822wrap_ty *)fp;
	input_pushback_transfer(this_thing->deeper, fp);
	if (this_thing->close_on_delete)
		input_delete(this_thing->deeper);
}


static long
input_822wrap_read(input_ty *fp, void *data, size_t len)
{
	input_822wrap_ty *this_thing;
	int		c;
	unsigned char	*cp;
	unsigned char	*end;
	size_t		nbytes;

	this_thing = (input_822wrap_ty *)fp;
	cp = (unsigned char *)data;
	end = cp + len;
	while (cp < end)
	{
		c = input_getc(this_thing->deeper);
		if (c < 0)
			break;
		if (c == '\n' && this_thing->column > 0)
		{
			c = input_getc(this_thing->deeper);
			if (c != ' ' && c != '\t')
			{
				if (c >= 0)
					input_ungetc(this_thing->deeper, c);
				c = '\n';
			}
		}
		*cp++ = c;
		if (c == '\n')
		{
			/*
			 * Line buffered, because we don't want to get
			 * too far ahead of ourselves, in case the header
			 * finishes and the binary data starts.
			 */
			this_thing->column = 0;
			break;
		}
		this_thing->column++;
	}
	nbytes = (cp - (unsigned char *)data);
	this_thing->pos += nbytes;
	return nbytes;
}


static long
input_822wrap_ftell(input_ty *fp)
{
	input_822wrap_ty *this_thing;

	this_thing = (input_822wrap_ty *)fp;
	return this_thing->pos;
}


static struct string_ty *
input_822wrap_name(input_ty *fp)
{
	input_822wrap_ty *this_thing;

	this_thing = (input_822wrap_ty *)fp;
	return input_name(this_thing->deeper);
}


static long
input_822wrap_length(input_ty *fp)
{
	return -1;
}


static input_vtbl_ty vtbl =
{
	sizeof(input_822wrap_ty),
	input_822wrap_destructor,
	input_822wrap_read,
	input_822wrap_ftell,
	input_822wrap_name,
	input_822wrap_length,
};


input_ty *
input_822wrap(input_ty *deeper, int close_on_delete)
{
	input_ty	*result;
	input_822wrap_ty *this_thing;

	result = input_new(&vtbl);
	this_thing = (input_822wrap_ty *)result;
	this_thing->deeper = deeper;
	this_thing->close_on_delete = close_on_delete;
	this_thing->pos = 0;
	this_thing->column = 0;
	return result;
}
