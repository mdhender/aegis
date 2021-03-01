//
//      aegis - project change supervisor
//      Copyright (C) 1999-2006, 2008, 2012 Peter Miller
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
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/limits.h>
#include <common/ac/stddef.h>
#include <common/ac/stdlib.h>
#include <common/ac/wchar.h>
#include <common/ac/wctype.h>

#include <common/language.h>
#include <common/str.h>
#include <common/trace.h>
#include <libaegis/wide_output/to_narrow.h>


wide_output_to_narrow::~wide_output_to_narrow()
{
    //
    // There could be outstanding state to flush.
    //
    trace(("wide_output_to_narrow::destructor(this = %p)\n{\n", this));
    flush();
    language_human();
    char buf[MB_LEN_MAX + 1];
    int n = wcrtomb(buf, (wchar_t)0, &state);
    language_C();
    // The last one should be a NUL.
    if (n > 0 && buf[n - 1] == 0)
        --n;
    if (n > 0)
        deeper->write(buf, n);
    trace(("}\n"));
}


static mbstate_t initial_state;


wide_output_to_narrow::wide_output_to_narrow(const output::pointer &a_deeper) :
    deeper(a_deeper),
    state(initial_state),
    prev_was_newline(true)
{
}


wide_output::pointer
wide_output_to_narrow::open(const output::pointer &a_deeper)
{
    return pointer(new wide_output_to_narrow(a_deeper));
}


nstring
wide_output_to_narrow::filename()
{
    return deeper->filename();
}


void
wide_output_to_narrow::write_inner(const wchar_t *data, size_t len)
{
    //
    // This is very similar to the single character case, however
    // we minimize the number of locale changes.
    //
    trace(("wide_output_to_narrow::write_inner(this = %p, data = %p, "
        "len = %ld)\n{\n", this, data, (long)len));
    while (len > 0)
    {
        char buf[2000];
        language_human();
        size_t buf_pos = 0;
        while (buf_pos + MB_LEN_MAX <= sizeof(buf) && len > 0)
        {
            trace(("data = %p, len = %ld\n", data, (long)len));
            wchar_t wc = *data++;
            --len;
            mbstate_t sequester = state;
            //
            // this could be recoded to use the wcsrtombs function, now
            // that it has sensible semantics around errors.
            //
            int n = wcrtomb(buf + buf_pos, wc, &state);
            if (n == -1)
            {
                state = sequester;
                language_C();
                deeper->write(buf, buf_pos);
                buf_pos = 0;
                deeper->fprintf("\\x%lX", (unsigned long)wc);
                language_human();
            }
            else
                buf_pos += n;
            prev_was_newline = (wc == L'\n');
        }
        language_C();
        deeper->write(buf, buf_pos);
        buf_pos = 0;
    }
    trace(("}\n"));
}


void
wide_output_to_narrow::flush_inner()
{
    deeper->flush();
}


int
wide_output_to_narrow::page_width()
{
    return deeper->page_width();
}


int
wide_output_to_narrow::page_length()
{
    return deeper->page_length();
}


void
wide_output_to_narrow::end_of_line_inner()
{
    trace(("wide_output_to_narrow::end_of_line_inner(this = %p)\n{\n",
        this));
    if (!prev_was_newline)
        put_wc(L'\n');
    trace(("}\n"));
}


const char *
wide_output_to_narrow::type_name()
    const
{
    return "wide_output_to_narrow";
}


// vim: set ts=8 sw=4 et :
