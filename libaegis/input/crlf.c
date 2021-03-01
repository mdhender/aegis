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
 * MANIFEST: functions for reading input and filtering out CR LF sequences
 */

#include <error.h>
#include <input/crlf.h>
#include <input/private.h>
#include <trace.h>


typedef struct input_crlf_ty input_crlf_ty;
struct input_crlf_ty
{
	input_ty	inherited;
	input_ty	*fp;
	long		pos;
	int		delete_on_close;
};


static void destruct _((input_ty *));

static void
destruct(p)
	input_ty	*p;
{
	input_crlf_ty	*this;

	trace(("input_crlf::destruct()\n{\n"));
	this = (input_crlf_ty *)p;
	input_pushback_transfer(this->fp, p);
	if (this->delete_on_close)
		input_delete(this->fp);
	this->fp = 0; /* paranoia */
	trace(("}\n"));
}


static int get _((input_ty *));

static int
get(p)
	input_ty	*p;
{
	input_crlf_ty	*this;
	int		c;

	trace(("input_crlf::get()\n{\n"));
	this = (input_crlf_ty *)p;
	c = input_getc(this->fp);
	if (c == '\r')
	{
		c = input_getc(this->fp);
		if (c != '\n')
		{
			input_ungetc(this->fp, c);
			c = '\r';
		}
	}
	if (c >= 0)
		this->pos++;
#ifdef DEBUG
	if (c < 0)
		trace(("return EOF;\n"));
	else if (c >= ' ' && c <= '~')
		trace(("return '%c';\n", c));
	else
		trace(("return 0x%02X;\n", c));
#endif
	trace(("}\n"));
	return c;
}


static long itell _((input_ty *));

static long
itell(fp)
	input_ty	*fp;
{
	input_crlf_ty	*this;

	this = (input_crlf_ty *)fp;
	trace(("input_crlf::tell => %ld\n", this->pos));
	return this->pos;
}


static const char *name _((input_ty *));

static const char *
name(p)
	input_ty	*p;
{
	input_crlf_ty	*this;

	trace(("input_crlf::name\n"));
	this = (input_crlf_ty *)p;
	return input_name(this->fp);
}


static long length _((input_ty *));

static long
length(p)
	input_ty	*p;
{
	trace(("input_crlf::length => -1\n"));
	return -1;
}


static input_vtbl_ty vtbl =
{
	sizeof(input_crlf_ty),
	destruct,
	input_generic_read,
	get,
	itell,
	name,
	length,
};


input_ty *
input_crlf(fp, delete_on_close)
	input_ty	*fp;
	int		delete_on_close;
{
	input_ty	*result;
	input_crlf_ty	*this;

	trace(("input_crlf(fp = %08lX)\n{\n", (long)fp));
	result = input_new(&vtbl);
	this = (input_crlf_ty *)result;
	this->fp = fp;
	this->pos = 0;
	this->delete_on_close = delete_on_close;
	trace(("return %08lX\n", (long)result));
	trace(("}\n"));
	return result;
}
