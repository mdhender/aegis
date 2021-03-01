//
//      aegis - project change supervisor
//      Copyright (C) 1999-2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/wchar.h>
#include <common/ac/wctype.h>

#include <common/language.h>
#include <common/str.h>
#include <common/trace.h>
#include <libaegis/wide_output/wrap.h>


void
wide_output_wrap::wrap()
{
    trace(("wide_output_wrap::wrap(this = %p)\n{\n", this));
    const wchar_t *s = buf.get_data();
    const wchar_t *s_end = s + buf.size();
    while (s < s_end)
    {
        //
        // remember where the line starts within the buffer
        //
        const wchar_t *s_start = s;
        int s_wid = 0;

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
            int c_wid = s_wid + wcwidth(*s);
            if (c_wid > width)
                break;
            ++s;
            s_wid = c_wid;
        }
        trace(("s_wid=%d width=%d\n", s_wid, width));

        //
        // If we reached the end of the line,
        // write it out and stop.
        //
        if (s >= s_end)
        {
            trace(("s_start=%p s=%p\n", s_start, s));
            language_C();
            deeper->write(s_start, s - s_start);
            break;
        }

        //
        // The line needs to be wrapped.
        // See if there is a better place to wrap it.
        //
        trace(("mark\n"));
        if (s < s_end && !iswspace(*s))
        {
            const wchar_t *s2 = s;
            while
            (
                s2 > s_start
            &&
                !iswspace(s2[-1])
            &&
                s2[-1] != L'-'
            &&
                s2[-1] != L'_'
            &&
                s2[-1] != L'/'
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
        trace(("s_start=%p s=%p\n", s_start, s));
        language_C();
        deeper->write(s_start, s - s_start);
        while (s < s_end && iswspace(*s))
            ++s;
        trace(("s=%p\n", s));
        if (s >= s_end)
            break;
        deeper->put_wc(L'\n');
    }

    //
    // End the line with a newline, even if the input didn't have one.
    //
    deeper->put_wc(L'\n');

    //
    // Reset the line, now that we've written it out.
    //
    buf.clear();
    trace(("}\n"));
}


wide_output_wrap::~wide_output_wrap()
{
    trace(("wide_output_wrap::destructor(this = %p)\n{\n", this));
    flush();
    if (!buf.empty())
        wrap();
    trace(("}\n"));
}


wide_output_wrap::wide_output_wrap(const wide_output::pointer &a_deeper,
        int a_width) :
    deeper(a_deeper),
    width(a_width <= 0 ? a_deeper->page_width() : a_width)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


wide_output::pointer
wide_output_wrap::open(const wide_output::pointer &a_deeper, int a_width)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new wide_output_wrap(a_deeper, a_width));
}


nstring
wide_output_wrap::filename()
{
    return deeper->filename();
}


void
wide_output_wrap::write_inner(const wchar_t *data, size_t len)
{
    trace(("wide_output_wrap::write(this = %p, data = %p, "
        "len = %ld)\n{\n", this, data, (long)len));
    while (len > 0)
    {
        wchar_t wc = *data++;
        --len;
        if (wc == L'\n')
            wrap();
        else
            buf.push_back(wc);
    }
    trace(("}\n"));
}


void
wide_output_wrap::flush_inner()
{
    deeper->flush();
}


int
wide_output_wrap::page_width()
{
    return width;
}


int
wide_output_wrap::page_length()
{
    return deeper->page_length();
}


void
wide_output_wrap::end_of_line_inner()
{
    trace(("wide_output_wrap::eoln(this = %p)\n{\n", this));
    if (!buf.empty())
        put_wc(L'\n');
    trace(("}\n"));
}


const char *
wide_output_wrap::type_name()
    const
{
    return "wide_output_wrap";
}


// vim: set ts=8 sw=4 et :
