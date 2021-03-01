//
// aegis - project change supervisor
// Copyright (C) 2005, 2006, 2008, 2011, 2012 Peter Miller
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

#include <aerevml/output/revml_encode.h>


output_revml_encode::~output_revml_encode()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();
}


output_revml_encode::output_revml_encode(const output::pointer &a_deeper) :
    deeper(a_deeper),
    pos(0),
    bol(true)
{
}


output::pointer
output_revml_encode::create(const output::pointer &a_deeper)
{
    return pointer(new output_revml_encode(a_deeper));
}


static char
hex(unsigned x)
{
    return "0123456789ABCDEF"[x & 15];
}


void
output_revml_encode::write_inner(const void *data, size_t length)
{
    unsigned char *cp = (unsigned char *)data;
    while (length > 0)
    {
        unsigned char c = *cp++;
        --length;
        ++pos;
        bol = false;
        switch (c)
        {
        case '\n':
            bol = true;
            // fall through...

        case ' ':
        case '\t':
            deeper->fputc(c);
            break;

        case '&':
            deeper->fputs("&amp;");
            break;

        case '"':
            deeper->fputs("&quot;");
            break;

        case '\'':
            deeper->fputs("&apos;");
            break;

        case '<':
            deeper->fputs("&lt;");
            break;

        case '>':
            deeper->fputs("&gt;");
            break;

        default:
            // C locale
            if (isprint(c))
                deeper->fputc(c);
            else if (c < ' ')
                deeper->fprintf("<char code=\"0x%c%c\"/>", hex(c >> 4), hex(c));
            else
                deeper->fprintf("&#%d;", c);
            break;
        }
    }
}


void
output_revml_encode::flush_inner(void)
{
    deeper->flush();
}


nstring
output_revml_encode::filename(void)
    const
{
    return deeper->filename();
}


long
output_revml_encode::ftell_inner(void)
    const
{
    return pos;
}


int
output_revml_encode::page_width(void)
    const
{
    return deeper->page_width();
}


int
output_revml_encode::page_length(void)
    const
{
    return deeper->page_length();
}


void
output_revml_encode::end_of_line_inner(void)
{
    if (!bol)
        fputc('\n');
}


nstring
output_revml_encode::type_name(void)
    const
{
    return ("revml > " + deeper->type_name());
}


// vim: set ts=8 sw=4 et :
