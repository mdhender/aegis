/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999-2001 Peter Miller;
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
 * MANIFEST: functions to manipulate heads
 *
 * This class of wide output is used to preserve the first few lines of
 * a stream, and discard the rest.  It is similar to the head(1) command.
 */

#include <str.h>
#include <wide_output/head.h>
#include <wide_output/private.h>
#include <trace.h>


typedef struct wide_output_head_ty wide_output_head_ty;
struct wide_output_head_ty
{
	wide_output_ty	inherited;
	wide_output_ty	*deeper;
	int		delete_on_close;
	int		how_many_lines;
	int		prev_was_newline;
};


static void wide_output_head_destructor _((wide_output_ty *));

static void
wide_output_head_destructor(fp)
	wide_output_ty	*fp;
{
	wide_output_head_ty *this;

	trace(("wide_output_head_destructor(fp = %08lX)\n{\n", (long)fp));
	this = (wide_output_head_ty *)fp;
	if (this->delete_on_close)
		wide_output_delete(this->deeper);
	this->deeper = 0;
	trace(("}\n"));
}


static string_ty *wide_output_head_filename _((wide_output_ty *));

static string_ty *
wide_output_head_filename(fp)
	wide_output_ty	*fp;
{
	wide_output_head_ty *this;

	this = (wide_output_head_ty *)fp;
	return wide_output_filename(this->deeper);
}


static void wide_output_head_write _((wide_output_ty *, const wchar_t *,
	size_t));

static void
wide_output_head_write(fp, data, len)
	wide_output_ty	*fp;
	const wchar_t	*data;
	size_t		len;
{
	wide_output_head_ty *this;

	trace(("wide_output_head_write(fp = %08lX, date = %08lX, \
len = %ld)\n{\n", (long)fp, (long)data, (long)len));
	this = (wide_output_head_ty *)fp;
	while (this->how_many_lines > 0 && len > 0)
	{
		wchar_t wc = *data++;
		--len;

		wide_output_putwc(this->deeper, wc);
		this->prev_was_newline = (wc == (wchar_t)'\n');
		if (this->prev_was_newline)
			this->how_many_lines--;
	}
	trace(("}\n"));
}


static void wide_output_head_flush _((wide_output_ty *));

static void
wide_output_head_flush(fp)
	wide_output_ty	*fp;
{
	wide_output_head_ty *this;

	trace(("wide_output_head_flush(fp = %08lX)\n{\n", (long)fp));
	this = (wide_output_head_ty *)fp;
	wide_output_flush(this->deeper);
	trace(("}\n"));
}


static int wide_output_head_page_width _((wide_output_ty *));

static int
wide_output_head_page_width(fp)
	wide_output_ty	*fp;
{
	wide_output_head_ty *this;

	this = (wide_output_head_ty *)fp;
	return wide_output_page_width(this->deeper);
}


static int wide_output_head_page_length _((wide_output_ty *));

static int
wide_output_head_page_length(fp)
	wide_output_ty	*fp;
{
	wide_output_head_ty *this;

	this = (wide_output_head_ty *)fp;
	return wide_output_page_length(this->deeper);
}


static void wide_output_head_eoln _((wide_output_ty *));

static void
wide_output_head_eoln(fp)
	wide_output_ty	*fp;
{
	wide_output_head_ty *this;

	trace(("wide_output_head_eoln(fp = %08lX)\n{\n", (long)fp));
	this = (wide_output_head_ty *)fp;
	if (!this->prev_was_newline)
		wide_output_putwc(fp, (wchar_t)'\n');
	trace(("}\n"));
}


static wide_output_vtbl_ty vtbl =
{
	sizeof(wide_output_head_ty),
	wide_output_head_destructor,
	wide_output_head_filename,
	wide_output_head_write,
	wide_output_head_flush,
	wide_output_head_page_width,
	wide_output_head_page_length,
	wide_output_head_eoln,
	"head",
};


wide_output_ty *
wide_output_head_open(deeper, delete_on_close, nlines)
	wide_output_ty	*deeper;
	int		delete_on_close;
	int		nlines;
{
	wide_output_ty	*result;
	wide_output_head_ty *this;

	trace(("wide_output_head_open(deeper = %08lX, nlines = %d)\n{\n",
		(long)deeper, nlines));
	result = wide_output_new(&vtbl);
	this = (wide_output_head_ty *)result;
	this->deeper = deeper;
	this->delete_on_close = delete_on_close;
	this->how_many_lines = (nlines <= 0 ? 1 : nlines);
	this->prev_was_newline = 1;
	trace(("return %08lX;\n", (long)result));
	trace(("}\n"));
	return result;
}