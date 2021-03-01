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
 * MANIFEST: functions to send wide char output into narrow char output
 *
 * This class of wide output is used to convert to a wide character output
 * stream into a multi-byte encoded "narrow character" output stream.
 */

#include <ac/limits.h>
#include <ac/stddef.h>
#include <ac/stdlib.h>
#include <ac/wchar.h>
#include <ac/wctype.h>

#include <language.h>
#include <str.h>
#include <trace.h>
#include <wide_output/private.h>
#include <wide_output/to_narrow.h>


typedef struct wide_output_to_narrow_ty wide_output_to_narrow_ty;
struct wide_output_to_narrow_ty
{
	wide_output_ty	inherited;
	output_ty	*deeper;
	int		delete_on_close;
	mbstate_t	state;					/*lint !e43*/
	int		prev_was_newline;
};


static void wide_output_to_narrow_destructor _((wide_output_ty *));

static void
wide_output_to_narrow_destructor(fp)
    wide_output_ty	*fp;
{
	wide_output_to_narrow_ty *this;
	char		buf[MB_LEN_MAX + 1];
	int		n;

	trace(("wide_output_to_narrow::destructor(fp = %08lX)\n{\n", (long)fp));
	this = (wide_output_to_narrow_ty *)fp;

	/*
	 * There could be outstanding state to flush.
	 */
	language_human();
	n = wcrtomb(buf, (wchar_t)0, &this->state);
	language_C();
	/* The last one should be a NUL.  */
	if (n > 0 && buf[n - 1] == 0)
		--n;
	if (n > 0)
		output_write(this->deeper, buf, n);

	/*
	 * Delete the deeper output on close if we were asked to.
	 */
	if (this->delete_on_close)
		output_delete(this->deeper);
	this->deeper = 0;
	trace(("}\n"));
}


static string_ty *wide_output_to_narrow_filename _((wide_output_ty *));

static string_ty *
wide_output_to_narrow_filename(fp)
	wide_output_ty	*fp;
{
	wide_output_to_narrow_ty *this;

	this = (wide_output_to_narrow_ty *)fp;
	return output_filename(this->deeper);
}


static void wide_output_to_narrow_write _((wide_output_ty *, const wchar_t *,
	size_t));

static void
wide_output_to_narrow_write(fp, data, len)
	wide_output_ty	*fp;
	const wchar_t	*data;
	size_t		len;
{
	wide_output_to_narrow_ty *this;
	int		n;
	size_t		buf_pos;
	wchar_t		wc;
	mbstate_t	sequester;				/*lint !e86*/
	char		buf[2000];

	/*
	 * This is very similar to the single character case, however
	 * we minimize the number of locale changes.
	 */
	trace(("wide_output_to_narrow::write(fp = %08lX, data = %08lX, len = %ld)\n{\n",
		(long)fp, (long)data, (long)len));
	this = (wide_output_to_narrow_ty *)fp;
	while (len > 0)
	{
		language_human();
		buf_pos = 0;
		while (buf_pos + MB_LEN_MAX <= sizeof(buf) && len > 0)
		{
			trace(("data = %08lX, len = %ld\n",
				(long)data, (long)len));
			wc = *data++;
			--len;
			sequester = this->state;
			n = wcrtomb(buf + buf_pos, wc, &this->state);
			if (n == -1)
			{
				this->state = sequester;
				language_C();
				output_write(this->deeper, buf, buf_pos);
				buf_pos = 0;
				output_fprintf(this->deeper, "\\x%lX", (unsigned long)wc);
				language_human();
			}
			else
				buf_pos += n;
			this->prev_was_newline = (wc == '\n');
		}
		language_C();
		output_write(this->deeper, buf, buf_pos);
		buf_pos = 0;
	}
	trace(("}\n"));
}


static void wide_output_to_narrow_flush _((wide_output_ty *));

static void
wide_output_to_narrow_flush(fp)
	wide_output_ty	*fp;
{
	wide_output_to_narrow_ty *this;

	this = (wide_output_to_narrow_ty *)fp;
	output_flush(this->deeper);
}


static int wide_output_to_narrow_page_width _((wide_output_ty *));

static int
wide_output_to_narrow_page_width(fp)
	wide_output_ty	*fp;
{
	wide_output_to_narrow_ty *this;

	this = (wide_output_to_narrow_ty *)fp;
	return output_page_width(this->deeper);
}


static int wide_output_to_narrow_page_length _((wide_output_ty *));

static int
wide_output_to_narrow_page_length(fp)
	wide_output_ty	*fp;
{
	wide_output_to_narrow_ty *this;

	this = (wide_output_to_narrow_ty *)fp;
	return output_page_length(this->deeper);
}


static void wide_output_to_narrow_eoln _((wide_output_ty *));

static void
wide_output_to_narrow_eoln(fp)
	wide_output_ty	*fp;
{
	wide_output_to_narrow_ty *this;

	trace(("wide_output_to_narrow::eoln(fp = %08lX)\n{\n", (long)fp));
	this = (wide_output_to_narrow_ty *)fp;
	if (!this->prev_was_newline)
		wide_output_putwc(fp, (wchar_t)'\n');
	trace(("}\n"));
}


static wide_output_vtbl_ty vtbl =
{
	sizeof(wide_output_to_narrow_ty),
	wide_output_to_narrow_destructor,
	wide_output_to_narrow_filename,
	wide_output_to_narrow_write,
	wide_output_to_narrow_flush,
	wide_output_to_narrow_page_width,
	wide_output_to_narrow_page_length,
	wide_output_to_narrow_eoln,
	"to_narrow",
};


wide_output_ty *
wide_output_to_narrow_open(deeper, delete_on_close)
	output_ty	*deeper;
	int		delete_on_close;
{
	wide_output_ty	*result;
	wide_output_to_narrow_ty *this;
	static mbstate_t initial_state;				/*lint !e86*/

	trace(("wide_output_to_narrow::new(deeper = %08lX)\n{\n",
		(long)deeper));
	result = wide_output_new(&vtbl);
	this = (wide_output_to_narrow_ty *)result;
	this->deeper = deeper;
	this->delete_on_close = delete_on_close;
	this->state = initial_state;
	this->prev_was_newline = 1;
	trace(("return %08lX;\n", (long)result));
	trace(("}\n"));
	return result;
}
