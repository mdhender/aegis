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


static void destructor _((input_ty *));

static void
destructor(fp)
	input_ty	*fp;
{
	input_822wrap_ty *this;

	this = (input_822wrap_ty *)fp;
	input_pushback_transfer(this->deeper, fp);
	if (this->close_on_delete)
		input_delete(this->deeper);
}


static int iget _((input_ty *));

static int
iget(fp)
	input_ty	*fp;
{
	input_822wrap_ty *this;
	int		c;

	this = (input_822wrap_ty *)fp;
	c = input_getc(this->deeper);
	if (c < 0)
		return -1;
	if (c == '\n' && this->column > 0)
	{
		c = input_getc(this->deeper);
		if (c != ' ' && c != '\t')
		{
			input_ungetc(this->deeper, c);
			c = '\n';
		}
	}
	this->pos++;
	if (c == '\n')
		this->column = 0;
	else
		this->column++;
	return c;
}


static long itell _((input_ty *));

static long
itell(fp)
	input_ty	*fp;
{
	input_822wrap_ty *this;

	this = (input_822wrap_ty *)fp;
	return this->pos;
}


static const char *iname _((input_ty *));

static const char *
iname(fp)
	input_ty	*fp;
{
	input_822wrap_ty *this;

	this = (input_822wrap_ty *)fp;
	return input_name(this->deeper);
}


static long ilength _((input_ty *));

static long
ilength(fp)
	input_ty	*fp;
{
	return -1;
}


static input_vtbl_ty vtbl =
{
	sizeof(input_822wrap_ty),
	destructor,
	input_generic_read,
	iget,
	itell,
	iname,
	ilength,
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
