//
//	aegis - project change supervisor
//	Copyright (C) 1999-2004 Peter Miller;
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
// MANIFEST: functions to manipulate expands
//
// This class of wide output is used to replace tabs with spaces.  This
// simplifies a lot of the internal width calculations for things like
// truncating, wrapping and reassembling.  This internal filtering uses
// 8-character tabs; the -tw option is exclusively for output tab widths.
//

#include <ac/wchar.h>
#include <ac/wctype.h>

#include <language.h>
#include <mem.h>
#include <str.h>
#include <trace.h>
#include <wide_output.h>
#include <wide_output/expand.h>
#include <wide_output/private.h>


typedef struct wide_output_expand_ty wide_output_expand_ty;
struct wide_output_expand_ty
{
	wide_output_ty	inherited;
	wide_output_ty	*deeper;
	int		delete_on_close;
	int		column;
};


static void
wide_output_expand_destructor(wide_output_ty *fp)
{
	wide_output_expand_ty *this_thing;

	trace(("wide_output_expand_destructor(fp = %08lX)\n{\n", (long)fp));
	this_thing = (wide_output_expand_ty *)fp;
	if (this_thing->delete_on_close)
		wide_output_delete(this_thing->deeper);
	this_thing->deeper = 0;
	trace(("}\n"));
}


static string_ty *
wide_output_expand_filename(wide_output_ty *fp)
{
	wide_output_expand_ty *this_thing;

	this_thing = (wide_output_expand_ty *)fp;
	return wide_output_filename(this_thing->deeper);
}


static void
hex(wide_output_expand_ty *this_thing, int n)
{
	wide_output_putwc(this_thing->deeper,
                          (wchar_t)("0123456789ABCDEF"[n & 15]));
}


static void
wide_output_expand_write(wide_output_ty *fp, const wchar_t *data, size_t len)
{
	wide_output_expand_ty *this_thing;

	trace(("wide_output_expand_write(fp = %08lX, data = %08lX, \
len = %ld)\n{\n", (long)fp, (long)data, (long)len));
	this_thing = (wide_output_expand_ty *)fp;
	while (len > 0)
	{
		wchar_t wc = *data++;
		--len;

		switch (wc)
		{
		case (wchar_t)'\n':
		case (wchar_t)'\f':
			wide_output_putwc(this_thing->deeper, wc);
			this_thing->column = 0;
			break;

		case (wchar_t)'\t':
			// internally, treat tabs as 8 characters wide
			for (;;)
			{
				wide_output_putwc(this_thing->deeper,
                                                  (wchar_t)' ');
				this_thing->column++;
				if (!(this_thing->column & 7))
					break;
			}
			break;

		case (wchar_t)0:
		case (wchar_t)' ':
			wide_output_putwc(this_thing->deeper, (wchar_t)' ');
			this_thing->column++;
			break;

		default:
			language_human();
			if (!iswprint(wc))
			{
				language_C();
				wide_output_putwc(this_thing->deeper,
                                                  (wchar_t)'\\');
				wide_output_putwc(this_thing->deeper,
                                                  (wchar_t)'x');
				hex(this_thing, (int)(wc >> 12));
				hex(this_thing, (int)(wc >>  8));
				hex(this_thing, (int)(wc >>  4));
				hex(this_thing, (int)(wc      ));
				this_thing->column += 6;
			}
			else
			{
				this_thing->column += wcwidth(wc);
				language_C();
				wide_output_putwc(this_thing->deeper, wc);
			}
			break;
		}
	}
	trace(("}\n"));
}


static void
wide_output_expand_flush(wide_output_ty *fp)
{
	wide_output_expand_ty *this_thing;

	trace(("wide_output_expand_flush(fp = %08lX)\n{\n", (long)fp));
	this_thing = (wide_output_expand_ty *)fp;
	wide_output_flush(this_thing->deeper);
	trace(("}\n"));
}


static int
wide_output_expand_page_width(wide_output_ty *fp)
{
	wide_output_expand_ty *this_thing;
	int		result;

	trace(("wide_output_expand_page_width(fp = %08lX)\n{\n", (long)fp));
	this_thing = (wide_output_expand_ty *)fp;
	result = wide_output_page_width(this_thing->deeper);
	trace(("return %d;\n", result));
	trace(("}\n"));
	return result;
}


static int
wide_output_expand_page_length(wide_output_ty *fp)
{
	wide_output_expand_ty *this_thing;
	int		result;

	trace(("wide_output_expand_page_length(fp = %08lX)\n{\n", (long)fp));
	this_thing = (wide_output_expand_ty *)fp;
	result = wide_output_page_length(this_thing->deeper);
	trace(("return %d;\n", result));
	trace(("}\n"));
	return result;
}


static void
wide_output_expand_eoln(wide_output_ty *fp)
{
	wide_output_expand_ty *this_thing;

	trace(("wide_output_expand_eoln(fp = %08lX)\n{\n", (long)fp));
	this_thing = (wide_output_expand_ty *)fp;
	if (this_thing->column > 0)
		wide_output_putwc(fp, (wchar_t)'\n');
	trace(("}\n"));
}


static wide_output_vtbl_ty vtbl =
{
	sizeof(wide_output_expand_ty),
	wide_output_expand_destructor,
	wide_output_expand_filename,
	wide_output_expand_write,
	wide_output_expand_flush,
	wide_output_expand_page_width,
	wide_output_expand_page_length,
	wide_output_expand_eoln,
	"expand",
};


wide_output_ty *
wide_output_expand_open(wide_output_ty *deeper, int delete_on_close)
{
	wide_output_ty	*result;
	wide_output_expand_ty *this_thing;

	trace(("wide_output_expand_open(deeper = %08lX)\n{\n", (long)deeper));
	result = wide_output_new(&vtbl);
	this_thing = (wide_output_expand_ty *)result;
	this_thing->deeper = deeper;
	this_thing->delete_on_close = delete_on_close;
	this_thing->column = 0;
	trace(("return %08lX;\n", (long)result));
	trace(("}\n"));
	return result;
}
