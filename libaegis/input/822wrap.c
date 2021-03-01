/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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


static void input_822wrap_destructor _((input_ty *));

static void
input_822wrap_destructor(fp)
	input_ty	*fp;
{
	input_822wrap_ty *this;

	this = (input_822wrap_ty *)fp;
	input_pushback_transfer(this->deeper, fp);
	if (this->close_on_delete)
		input_delete(this->deeper);
}


static long input_822wrap_read _((input_ty *, void *, size_t));

static long
input_822wrap_read(fp, data, len)
	input_ty	*fp;
	void		*data;
	size_t		len;
{
	input_822wrap_ty *this;
	int		c;
	unsigned char	*cp;
	unsigned char	*end;
	size_t		nbytes;

	this = (input_822wrap_ty *)fp;
	cp = data;
	end = cp + len;
	while (cp < end)
	{
		c = input_getc(this->deeper);
		if (c < 0)
			break;
		if (c == '\n' && this->column > 0)
		{
			c = input_getc(this->deeper);
			if (c != ' ' && c != '\t')
			{
				if (c >= 0)
					input_ungetc(this->deeper, c);
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
			this->column = 0;
			break;
		}
		this->column++;
	}
	nbytes = (cp - (unsigned char *)data);
	this->pos += nbytes;
	return nbytes;
}


static long input_822wrap_ftell _((input_ty *));

static long
input_822wrap_ftell(fp)
	input_ty	*fp;
{
	input_822wrap_ty *this;

	this = (input_822wrap_ty *)fp;
	return this->pos;
}


static struct string_ty *input_822wrap_name _((input_ty *));

static struct string_ty *
input_822wrap_name(fp)
	input_ty	*fp;
{
	input_822wrap_ty *this;

	this = (input_822wrap_ty *)fp;
	return input_name(this->deeper);
}


static long input_822wrap_length _((input_ty *));

static long
input_822wrap_length(fp)
	input_ty	*fp;
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
input_822wrap(deeper, close_on_delete)
	input_ty	*deeper;
	int		close_on_delete;
{
	input_ty	*result;
	input_822wrap_ty *this;

	result = input_new(&vtbl);
	this = (input_822wrap_ty *)result;
	this->deeper = deeper;
	this->close_on_delete = close_on_delete;
	this->pos = 0;
	this->column = 0;
	return result;
}
