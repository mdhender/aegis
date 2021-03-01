//
// aegis - project change supervisor
// Copyright (C) 2001, 2003-2006, 2008, 2011, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>

#include <common/trace.h>
#include <libaegis/output/filter/prefix.h>


output_filter_prefix::~output_filter_prefix()
{
    trace(("~output_filter_prefix(this = %p)\n{\n", this));
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();
    trace(("}\n"));
}


static unsigned
expanded_size_of(const nstring &text)
{
    const char *cp = text.c_str();
    unsigned col = 0;
    for (;;)
    {
        unsigned char c = *cp++;
        switch (c)
        {
        case '\0':
            return col;

        case '\t':
            col = (col + 8) & ~7;
            break;

        case '\n':
        case '\r':
            col = 0;
            break;

        case '\b':
            if (col > 0)
                --col;
            break;

        default:
            ++col;
            break;
        }
    }
}


static unsigned
subtract(unsigned a, unsigned b)
{
    if (a < b)
        return 0;
    return (a - b);
}


output_filter_prefix::output_filter_prefix(
    const output::pointer &a_deeper,
    const nstring &a_prefix
) :
    output_filter(a_deeper),
    prefix(a_prefix),
    prefix0(a_prefix.trim_right()),
    column(0),
    pos(0),
    width(subtract(a_deeper->page_width(), expanded_size_of(a_prefix)))
{
    trace(("output_filter_prefix(this = %p)\n", this));
}


output::pointer
output_filter_prefix::create(const output::pointer &a_deeper,
    const char *a_prefix)
{
    return pointer(new output_filter_prefix(a_deeper, a_prefix));
}


void
output_filter_prefix::write_inner(const void *p, size_t len)
{
    trace(("output_filter_prefix::write_inner(this = %p, p = %p, "
        "len = %ld)\n{\n", this, p, len));
    const unsigned char *data = (unsigned char *)p;
    const unsigned char *begin = data;
    while (len > 0)
    {
        unsigned char c = *data++;
        --len;

        if (column == 0)
        {
            deeper_fputs(c == '\n' ? prefix0 : prefix);
            pos += prefix.length();
        }
        if (c == '\n')
        {
            size_t nbytes = data - begin;
            deeper_write(begin, nbytes);
            column = 0;
            pos += nbytes;
            begin = data;
        }
        else
            ++column;
    }
    if (data > begin)
    {
        size_t nbytes = data - begin;
        deeper_write(begin, nbytes);
        pos += nbytes;
    }
    trace(("}\n"));
}


long
output_filter_prefix::ftell_inner(void)
    const
{
    return pos;
}


int
output_filter_prefix::page_width(void)
    const
{
    return width;
}


void
output_filter_prefix::end_of_line_inner(void)
{
    trace(("output_filter_prefix::end_of_line_inner(this = %p)\n{\n", this));
    if (column != 0)
        write_inner("\n", 1);
    trace(("}\n"));
}


nstring
output_filter_prefix::type_name(void)
    const
{
    return ("prefix " + output_filter::type_name());
}


// vim: set ts=8 sw=4 et :
