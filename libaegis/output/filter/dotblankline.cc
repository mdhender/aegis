//
// aegis - project change supervisor
// Copyright (C) 2008, 2011 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <libaegis/output/filter/dotblankline.h>


output_filter_dot_blank_lines::~output_filter_dot_blank_lines()
{
}


output_filter_dot_blank_lines::output_filter_dot_blank_lines(
    const pointer &a_deeper
) :
    output_filter(a_deeper),
    column(0),
    pos(0)
{
}


output::pointer
output_filter_dot_blank_lines::create(const pointer &a_deeper)
{
    return pointer(new output_filter_dot_blank_lines(a_deeper));
}


nstring
output_filter_dot_blank_lines::type_name(void)
    const
{
    return ("dot " + output_filter::type_name());
}


long
output_filter_dot_blank_lines::ftell_inner(void)
    const
{
    return pos;
}


void
output_filter_dot_blank_lines::write_inner(const void *data, size_t length)
{
    const char *start = (const char *)data;
    const char *end = start + length;
    const char *cp = start;
    while (cp < end)
    {
        unsigned char c = *cp++;
        if (c == '\n')
        {
            if (column == 0)
                deeper_fputc('.');
            column = 0;
        }
        else
            ++column;
        deeper_fputc(c);
    }
    pos += length;
}


void
output_filter_dot_blank_lines::end_of_line_inner(void)
{
    if (column > 0)
    {
        write_inner("\n", 1);
    }
}
