//
//      aegis - project change supervisor
//      Copyright (C) 1999-2001, 2003-2006, 2008, 2012 Peter Miller
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

#include <common/str.h>
#include <common/trace.h>
#include <libaegis/wide_output/head.h>


wide_output_head::~wide_output_head()
{
    trace(("wide_output_head::destructor(this = %p)\n{\n", this));
    flush();
    trace(("}\n"));
}


wide_output_head::wide_output_head(const wide_output::pointer &a_deeper,
        int a_nlines) :
    deeper(a_deeper),
    how_many_lines(a_nlines <= 0 ? 1 : a_nlines),
    prev_was_newline(true)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


wide_output::pointer
wide_output_head::open(const wide_output::pointer &a_deeper, int a_nlines)
{
    return pointer(new wide_output_head(a_deeper, a_nlines));
}


nstring
wide_output_head::filename()
{
    return deeper->filename();
}


void
wide_output_head::write_inner(const wchar_t *data, size_t len)
{
    trace(("wide_output_head::write(this = %p, data = %p, "
        "len = %ld)\n{\n", this, data, (long)len));
    while (how_many_lines > 0 && len > 0)
    {
        wchar_t wc = *data++;
        --len;

        deeper->put_wc(wc);
        prev_was_newline = (wc == L'\n');
        if (prev_was_newline)
            how_many_lines--;
    }
    trace(("}\n"));
}


void
wide_output_head::flush_inner()
{
    trace(("wide_output_head::flush(this = %p)\n{\n", this));
    deeper->flush();
    trace(("}\n"));
}


int
wide_output_head::page_width()
{
    return deeper->page_width();
}


int
wide_output_head::page_length()
{
    return deeper->page_length();
}


void
wide_output_head::end_of_line_inner()
{
    trace(("wide_output_head::end_of_line_inner(this = %p)\n{\n",
        this));
    if (!prev_was_newline)
        put_wc(L'\n');
    trace(("}\n"));
}


const char *
wide_output_head::type_name()
    const
{
    return "wide_output_head";
}


// vim: set ts=8 sw=4 et :
