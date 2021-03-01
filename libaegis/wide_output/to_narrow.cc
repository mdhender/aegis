//
//	aegis - project change supervisor
//	Copyright (C) 1999-2005 Peter Miller;
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
// MANIFEST: functions to send wide char output into narrow char output
//
// This class of wide output is used to convert to a wide character output
// stream into a multi-byte encoded "narrow character" output stream.
//

#include <common/ac/limits.h>
#include <common/ac/stddef.h>
#include <common/ac/stdlib.h>
#include <common/ac/wchar.h>
#include <common/ac/wctype.h>

#include <common/language.h>
#include <common/str.h>
#include <common/trace.h>
#include <libaegis/wide_output/private.h>
#include <libaegis/wide_output/to_narrow.h>


struct wide_output_to_narrow_ty
{
    wide_output_ty  inherited;
    output_ty	    *deeper;
    int		    delete_on_close;
    mbstate_t	    state;					//lint !e43
    int		    prev_was_newline;
};


static void
wide_output_to_narrow_destructor(wide_output_ty *fp)
{
    wide_output_to_narrow_ty *this_thing;
    char	    buf[MB_LEN_MAX + 1];
    int		    n;

    trace(("wide_output_to_narrow::destructor(fp = %08lX)\n{\n", (long)fp));
    this_thing = (wide_output_to_narrow_ty *)fp;

    //
    // There could be outstanding state to flush.
    //
    language_human();
    n = wcrtomb(buf, (wchar_t)0, &this_thing->state);
    language_C();
    // The last one should be a NUL.
    if (n > 0 && buf[n - 1] == 0)
	--n;
    if (n > 0)
	this_thing->deeper->write(buf, n);

    //
    // Delete the deeper output on close if we were asked to.
    //
    if (this_thing->delete_on_close)
	delete this_thing->deeper;
    this_thing->deeper = 0;
    trace(("}\n"));
}


static string_ty *
wide_output_to_narrow_filename(wide_output_ty *fp)
{
    wide_output_to_narrow_ty *this_thing;

    this_thing = (wide_output_to_narrow_ty *)fp;
    return this_thing->deeper->filename();
}


static void
wide_output_to_narrow_write(wide_output_ty *fp, const wchar_t *data, size_t len)
{
    wide_output_to_narrow_ty *this_thing;
    int		    n;
    size_t	    buf_pos;
    wchar_t	    wc;
    mbstate_t	    sequester;				//lint !e86
    char	    buf[2000];

    //
    // This is very similar to the single character case, however
    // we minimize the number of locale changes.
    //
    trace(("wide_output_to_narrow::write(fp = %08lX, data = %08lX, "
	"len = %ld)\n{\n", (long)fp, (long)data, (long)len));
    this_thing = (wide_output_to_narrow_ty *)fp;
    while (len > 0)
    {
	language_human();
	buf_pos = 0;
	while (buf_pos + MB_LEN_MAX <= sizeof(buf) && len > 0)
	{
	    trace(("data = %08lX, len = %ld\n", (long)data, (long)len));
	    wc = *data++;
	    --len;
	    sequester = this_thing->state;
	    n = wcrtomb(buf + buf_pos, wc, &this_thing->state);
	    if (n == -1)
	    {
		this_thing->state = sequester;
		language_C();
		this_thing->deeper->write(buf, buf_pos);
		buf_pos = 0;
		this_thing->deeper->fprintf("\\x%lX", (unsigned long)wc);
		language_human();
	    }
	    else
		buf_pos += n;
	    this_thing->prev_was_newline = (wc == '\n');
	}
	language_C();
	this_thing->deeper->write(buf, buf_pos);
	buf_pos = 0;
    }
    trace(("}\n"));
}


static void
wide_output_to_narrow_flush(wide_output_ty *fp)
{
    wide_output_to_narrow_ty *this_thing;

    this_thing = (wide_output_to_narrow_ty *)fp;
    this_thing->deeper->flush();
}


static int
wide_output_to_narrow_page_width(wide_output_ty *fp)
{
    wide_output_to_narrow_ty *this_thing;

    this_thing = (wide_output_to_narrow_ty *)fp;
    return this_thing->deeper->page_width();
}


static int
wide_output_to_narrow_page_length(wide_output_ty *fp)
{
    wide_output_to_narrow_ty *this_thing;

    this_thing = (wide_output_to_narrow_ty *)fp;
    return this_thing->deeper->page_length();
}


static void
wide_output_to_narrow_eoln(wide_output_ty *fp)
{
    wide_output_to_narrow_ty *this_thing;

    trace(("wide_output_to_narrow::eoln(fp = %08lX)\n{\n", (long)fp));
    this_thing = (wide_output_to_narrow_ty *)fp;
    if (!this_thing->prev_was_newline)
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
wide_output_to_narrow_open(output_ty *deeper, int delete_on_close)
{
    wide_output_ty  *result;
    wide_output_to_narrow_ty *this_thing;
    static mbstate_t initial_state;				//lint !e86

    trace(("wide_output_to_narrow::new(deeper = %08lX)\n{\n", (long)deeper));
    result = wide_output_new(&vtbl);
    this_thing = (wide_output_to_narrow_ty *)result;
    this_thing->deeper = deeper;
    this_thing->delete_on_close = delete_on_close;
    this_thing->state = initial_state;
    this_thing->prev_was_newline = 1;
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
