//
//	aegis - project change supervisor
//	Copyright (C) 1999-2006 Peter Miller
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
// MANIFEST: functions to manipulate unexpands
//
// This wide output class is used to convert sequences of spaces
// into tabs, where appropriate (and smaller than the original).
// The -Tab_Width option is used to determine the output tab width.
//

#include <common/ac/wchar.h>
#include <common/ac/wctype.h>

#include <common/language.h>
#include <common/mem.h>
#include <libaegis/option.h>
#include <common/str.h>
#include <common/trace.h>
#include <libaegis/wide_output.h>
#include <libaegis/wide_output/unexpand.h>
#include <libaegis/wide_output/private.h>


struct wide_output_unexpand_ty
{
    wide_output_ty  inherited;
    wide_output_ty  *deeper;
    int		    delete_on_close;
    int		    icol;
    int		    ocol;
    int		    tab_width;
};


static void
wide_output_unexpand_destructor(wide_output_ty *fp)
{
    wide_output_unexpand_ty *this_thing;

    trace(("wide_output_unexpand::destructor(fp = %08lX)\n{\n", (long)fp));
    this_thing = (wide_output_unexpand_ty *)fp;
    if (this_thing->delete_on_close)
	wide_output_delete(this_thing->deeper);
    this_thing->deeper = 0;
    trace(("}\n"));
}


static string_ty *
wide_output_unexpand_filename(wide_output_ty *fp)
{
    wide_output_unexpand_ty *this_thing;

    this_thing = (wide_output_unexpand_ty *)fp;
    return wide_output_filename(this_thing->deeper);
}


static void
wide_output_unexpand_write(wide_output_ty *fp, const wchar_t *data, size_t len)
{
    wide_output_unexpand_ty *this_thing;

    //
    // Put all of the output into a stash, rather then direct calls
    // to wide_output_putwc(this_thing->deeper).  This allows us to minimize
    // the number of language_C() and language_human() calls, which
    // tend to be slow-ish
    //
#define stash_size 1024
    wchar_t	    stash_buf[stash_size];
    wchar_t	    *stash_pos;
#define stash_end (stash_buf + stash_size)

#define stash(wc)							\
    { *stash_pos++ = (wc); if (stash_pos >= stash_end) { language_C();	\
    wide_output_write(this_thing->deeper, stash_buf, stash_size);	\
    language_human(); stash_pos = stash_buf; } }

    trace(("wide_output_unexpand::write(fp = %08lX, data = %08lX, "
	"len = %ld)\n{\n", (long)fp, (long)data, (long)len));
    this_thing = (wide_output_unexpand_ty *)fp;
    stash_pos = stash_buf;
    language_human();
    while (len > 0)
    {
	wchar_t		wc;

	wc = *data++;
	--len;
	switch (wc)
	{
	case (wchar_t)'\n':
	case (wchar_t)'\f':
	    stash(wc);
	    this_thing->icol = 0;
	    this_thing->ocol = 0;
	    break;

	case (wchar_t)'\t':
	    // internal tabs are 8 characters wide
	    this_thing->icol = (this_thing->icol + 8) & ~7;
	    break;

	case (wchar_t)0:
	case (wchar_t)' ':
	    this_thing->icol++;
	    break;

	default:
	    trace(("icol = %d\n", this_thing->icol));
	    if (this_thing->tab_width >= 2)
	    {
		trace(("tab_width = %d\n", this_thing->tab_width));
		for (;;)
		{
		    int		    ncol;

		    trace(("ocol = %d\n", this_thing->ocol));
		    if (this_thing->ocol + 1 >= this_thing->icol)
			break;
		    ncol =
			((this_thing->ocol / this_thing->tab_width) + 1) *
                        this_thing->tab_width;
		    trace(("ncol = %d\n", ncol));
		    if (ncol > this_thing->icol)
			break;
		    stash('\t');
		    this_thing->ocol = ncol;
		}
	    }
	    while (this_thing->ocol < this_thing->icol)
	    {
		trace(("ocol = %d\n", this_thing->ocol));
		stash(' ');
		this_thing->ocol++;
	    }
	    trace(("ocol = %d\n", this_thing->ocol));
	    this_thing->icol += wcwidth(wc);
	    stash(wc);
	    trace(("icol = %d\n", this_thing->icol));
	    this_thing->ocol = this_thing->icol;
	    break;
	}
    }
    language_C();
    if (stash_pos > stash_buf)
    {
	size_t		nwc;

	nwc = stash_pos - stash_buf;
	wide_output_write(this_thing->deeper, stash_buf, nwc);
    }
    trace(("}\n"));
}


static void
wide_output_unexpand_flush(wide_output_ty *fp)
{
    wide_output_unexpand_ty *this_thing;

    this_thing = (wide_output_unexpand_ty *)fp;
    wide_output_flush(this_thing->deeper);
}


static int
wide_output_unexpand_page_width(wide_output_ty *fp)
{
    wide_output_unexpand_ty *this_thing;

    this_thing = (wide_output_unexpand_ty *)fp;
    return wide_output_page_width(this_thing->deeper);
}


static int
wide_output_unexpand_page_length(wide_output_ty *fp)
{
    wide_output_unexpand_ty *this_thing;

    this_thing = (wide_output_unexpand_ty *)fp;
    return wide_output_page_length(this_thing->deeper);
}


static void
wide_output_unexpand_eoln(wide_output_ty *fp)
{
    wide_output_unexpand_ty *this_thing;

    trace(("wide_output_unexpand::eoln(fp = %08lX)\n{\n", (long)fp));
    this_thing = (wide_output_unexpand_ty *)fp;
    if (this_thing->icol > 0)
	wide_output_putwc(fp, (wchar_t)'\n');
    trace(("}\n"));
}


static wide_output_vtbl_ty vtbl =
{
    sizeof(wide_output_unexpand_ty),
    wide_output_unexpand_destructor,
    wide_output_unexpand_filename,
    wide_output_unexpand_write,
    wide_output_unexpand_flush,
    wide_output_unexpand_page_width,
    wide_output_unexpand_page_length,
    wide_output_unexpand_eoln,
    "unexpand",
};


wide_output_ty *
wide_output_unexpand_open(wide_output_ty *deeper, int delete_on_close,
    int tab_width)
{
    wide_output_ty  *result;
    wide_output_unexpand_ty *this_thing;

    trace(("wide_output_unexpand::new(deeper = %08lX)\n{\n", (long)deeper));
    result = wide_output_new(&vtbl);
    this_thing = (wide_output_unexpand_ty *)result;
    this_thing->deeper = deeper;
    this_thing->delete_on_close = delete_on_close;
    this_thing->icol = 0;
    this_thing->ocol = 0;
    this_thing->tab_width =
        (tab_width > 0 ? tab_width : option_tab_width_get());
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
