//
//      aegis - project change supervisor
//      Copyright (C) 1999-2006, 2008 Peter Miller
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

#include <common/language.h>
#include <libaegis/wide_output.h>
#include <libaegis/wide_output/truncate.h>


wide_output_truncate::~wide_output_truncate()
{
    flush();
    if (!buf.empty())
        deeper->write(buf.get_data(), buf.size());
}


wide_output_truncate::wide_output_truncate(const wide_output::pointer &a_deeper,
        int a_width) :
    deeper(a_deeper),
    width(a_width <= 0 ? a_deeper->page_width() : a_width),
    column(0)
{
}


wide_output::pointer
wide_output_truncate::open(const wide_output::pointer &a_deeper, int a_width)
{
    return pointer(new wide_output_truncate(a_deeper, a_width));
}


nstring
wide_output_truncate::filename()
{
    return deeper->filename();
}


void
wide_output_truncate::write_inner(const wchar_t *data, size_t len)
{
    language_human();
    while (len > 0)
    {
        wchar_t wc = *data++;
        --len;

        switch (wc)
        {
        case L'\n':
        case L'\f':
            language_C();
            if (!buf.empty())
            {
                deeper->write(buf.get_data(), buf.size());
                buf.clear();
            }
            deeper->put_wc(wc);
            language_human();
            column = 0;
            break;

        default:
            //
            // If we have already become too wide, don't
            // make the deeper unnecessary function calls.
            //
            if (column >= width)
                break;

            //
            // Only remember this_thing character if all of it
            // fits within the specified width.
            //
            int cwid = wcwidth(wc);
            if (column + cwid > width)
                break;

            //
            // Make room if necessary
            //
            buf.push_back(wc);
            column += cwid;
            break;
        }
    }
    language_C();
}


void
wide_output_truncate::flush_inner()
{
    deeper->write(buf.get_data(), buf.size());
    // DO NOT reset column
}


int
wide_output_truncate::page_width()
{
    return width;
}


int
wide_output_truncate::page_length()
{
    return deeper->page_length();
}


void
wide_output_truncate::end_of_line_inner()
{
    if (column > 0)
        put_wc(L'\n');
}


const char *
wide_output_truncate::type_name()
    const
{
    return "wide_output_truncate";
}
