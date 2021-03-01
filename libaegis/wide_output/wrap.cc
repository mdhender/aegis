//
//      aegis - project change supervisor
//      Copyright (C) 1999-2007 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate wraps
//
// This wide output class is used to wrap overly long lines.
//
// It DOES NOT catenate lines before wrapping, so it can't be used as
// a kind of fmt(1) equivalent.
//

#include <common/ac/wchar.h>
#include <common/ac/wctype.h>

#include <common/language.h>
#include <common/str.h>
#include <common/trace.h>
#include <libaegis/wide_output.h>
#include <libaegis/wide_output/private.h>
#include <libaegis/wide_output/wrap.h>


struct wide_output_wrap_ty
{
    wide_output_ty  inherited;
    wide_output_ty  *deeper;
    int             delete_on_close;
    int             width;

    wchar_t         *buf;
    size_t          buf_pos;
    size_t          buf_max;
};


static void
wrap(wide_output_wrap_ty *this_thing)
{
    wchar_t         *s;
    wchar_t         *s_end;

    trace(("wide_output_wrap::wrap(this_thing = %08lX)\n{\n",
           (long)this_thing));
    s = this_thing->buf;
    s_end = s + this_thing->buf_pos;
    while (s < s_end)
    {
        wchar_t         *s_start;
        int             s_wid;

        //
        // remember where the line starts within the buffer
        //
        s_start = s;
        s_wid = 0;

        //
        // Collect characters until we run out of width.
        //
        language_human();
        if (s < s_end && *s != '\n')
        {
            //
            // Always use the first character.  This avoids
            // an infinite loop where you have a 1-position
            // wide column, and a 2-position wide character.
            // (Or other variations on the same theme.)
            //
            s_wid += wcwidth(*s++);
        }
        while (s < s_end && *s != '\n')
        {
            int             c_wid;

            c_wid = s_wid + wcwidth(*s);
            if (c_wid > this_thing->width)
                break;
            ++s;
            s_wid = c_wid;
        }
        trace(("s_wid=%d width=%d\n", s_wid, this_thing->width));

        //
        // If we reached the end of the line,
        // write it out and stop.
        //
        if (s >= s_end)
        {
            trace(("s_start=%08lX s=%08lX\n", (long)s_start, (long)s));
            language_C();
            wide_output_write(this_thing->deeper, s_start, s - s_start);
            break;
        }

        //
        // The line needs to be wrapped.
        // See if there is a better place to wrap it.
        //
        trace(("mark\n"));
        if (s < s_end && !iswspace(*s))
        {
            wchar_t         *s2;

            s2 = s;
            while
            (
                s2 > s_start
            &&
                !iswspace(s2[-1])
            &&
                s2[-1] != (wchar_t)'-'
            &&
                s2[-1] != (wchar_t)'_'
            &&
                s2[-1] != (wchar_t)'/'
            )
                --s2;
            if (s2 > s_start)
            {
                while (s2 > s_start && iswspace(s2[-1]))
                    --s2;
                s = s2;
            }
        }

        //
        // Write out the line so far, plus the newline,
        // and then skip any trailing spaces (including any newlines).
        //
        trace(("s_start=%08lX s=%08lX\n", (long)s_start, (long)s));
        language_C();
        wide_output_write(this_thing->deeper, s_start, s - s_start);
        while (s < s_end && iswspace(*s))
            ++s;
        trace(("s=%08lX\n", (long)s));
        if (s >= s_end)
            break;
        wide_output_putwc(this_thing->deeper, (wchar_t)'\n');
    }

    //
    // End the line with a newline, even if the input didn't have one.
    //
    wide_output_putwc(this_thing->deeper, (wchar_t)'\n');

    //
    // Reset the line, now that we've written it out.
    //
    this_thing->buf_pos = 0;
    trace(("}\n"));
}


static void
wide_output_wrap_destructor(wide_output_ty *fp)
{
    wide_output_wrap_ty *this_thing;

    trace(("wide_output_wrap::destructor(fp = %08lX)\n{\n", (long)fp));
    this_thing = (wide_output_wrap_ty *)fp;
    if (this_thing->buf_pos)
        wrap(this_thing);
    delete [] this_thing->buf;
    if (this_thing->delete_on_close)
        wide_output_delete(this_thing->deeper);
    this_thing->deeper = 0;
    trace(("}\n"));
}


static string_ty *
wide_output_wrap_filename(wide_output_ty  *fp)
{
    wide_output_wrap_ty *this_thing;

    this_thing = (wide_output_wrap_ty *)fp;
    return wide_output_filename(this_thing->deeper);
}


static void
wide_output_wrap_write(wide_output_ty *fp, const wchar_t *data, size_t len)
{
    wide_output_wrap_ty *this_thing;

    trace(("wide_output_wrap::write(fp = %08lX, data = %08lX, len = %ld)\n{\n",
        (long)fp, (long)data, (long)len));
    this_thing = (wide_output_wrap_ty *)fp;
    while (len > 0)
    {
        wchar_t         wc;

        wc = *data++;
        --len;
        if (wc == (wchar_t)'\n')
            wrap(this_thing);
        else
        {
            if (this_thing->buf_pos >= this_thing->buf_max)
            {
                this_thing->buf_max = 16 + 2 * this_thing->buf_max;
		wchar_t *new_buf = new wchar_t [this_thing->buf_max];
		for (size_t k = 0; k < this_thing->buf_pos; ++k)
		    new_buf[k] = this_thing->buf[k];
		delete [] this_thing->buf;
		this_thing->buf = new_buf;
            }
            this_thing->buf[this_thing->buf_pos++] = wc;
        }
    }
    trace(("}\n"));
}


static void
wide_output_wrap_flush(wide_output_ty *fp)
{
    wide_output_wrap_ty *this_thing;

    this_thing = (wide_output_wrap_ty *)fp;
    wide_output_flush(this_thing->deeper);
}


static int
wide_output_wrap_page_width(wide_output_ty *fp)
{
    wide_output_wrap_ty *this_thing;

    this_thing = (wide_output_wrap_ty *)fp;
    return this_thing->width;
}


static int
wide_output_wrap_page_length(wide_output_ty *fp)
{
    wide_output_wrap_ty *this_thing;

    this_thing = (wide_output_wrap_ty *)fp;
    return wide_output_page_length(this_thing->deeper);
}


static void
wide_output_wrap_eoln(wide_output_ty *fp)
{
    wide_output_wrap_ty *this_thing;

    trace(("wide_output_wrap::eoln(fp = %08lX)\n{\n", (long)fp));
    this_thing = (wide_output_wrap_ty *)fp;
    if (this_thing->buf_pos > 0)
        wide_output_putwc(fp, (wchar_t)'\n');
    trace(("}\n"));
}


static wide_output_vtbl_ty vtbl =
{
    sizeof(wide_output_wrap_ty),
    wide_output_wrap_destructor,
    wide_output_wrap_filename,
    wide_output_wrap_write,
    wide_output_wrap_flush,
    wide_output_wrap_page_width,
    wide_output_wrap_page_length,
    wide_output_wrap_eoln,
    "wrap",
};


wide_output_ty *
wide_output_wrap_open(wide_output_ty *deeper, int delete_on_close, int width)
{
    wide_output_ty  *result;
    wide_output_wrap_ty *this_thing;

    trace(("wide_output_wrap::new(deeper = %08lX, doc = %d, width = %d)\n{\n",
        (long)deeper, delete_on_close, width));
    result = wide_output_new(&vtbl);
    this_thing = (wide_output_wrap_ty *)result;
    this_thing->deeper = deeper;
    this_thing->delete_on_close = delete_on_close;
    this_thing->width = (width <= 0 ? wide_output_page_width(deeper) : width);
    this_thing->buf = 0;
    this_thing->buf_pos = 0;
    this_thing->buf_max = 0;
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
