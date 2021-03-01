//
// aegis - project change supervisor
// Copyright (C) 2001-2006, 2008, 2011, 2012 Peter Miller
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>

#include <libaegis/output/filter/uuencode.h>
#include <common/str.h>

static char etab[] =
           "`!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";


output_filter_uuencode::~output_filter_uuencode()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    if (begun)
    {
        while (residual_bits > 0)
        {
            if (residual_bits < 6)
            {
                // fake a NUL character
                residual_value <<= 8;
                residual_bits += 8;
            }

            residual_bits -= 6;
            int n = (residual_value >> residual_bits);
            n &= 0x3F;
            obuf[opos++] = etab[n];
        }
        if (ipos)
        {
            obuf[0] = etab[ipos];
            obuf[opos++] = '\n';
            deeper_write(obuf, opos);
            ipos = 0;
            opos = 1; // The length character
        }
        deeper_fputs("`\nend\n");
    }
}


output_filter_uuencode::output_filter_uuencode(
    const output::pointer &a_deeper
) :
    output_filter(a_deeper),
    residual_value(0),
    residual_bits(0),
    opos(1), // The length character
    ipos(0),
    pos(0),
    bol(true),
    begun(false)
{
    //
    // Figure out what to call ourself,
    // discarding any ".uu*" suffix.
    //
    nstring fn = a_deeper->filename();
    const char *cp1 = fn.c_str();
    const char *cp2 = strrchr(cp1, '/');
    if (cp2)
        ++cp2;
    else
        cp2 = cp1;
    const char *cp3 = strstr(cp2, ".uu");
    if (cp3)
        file_name = nstring(cp1, cp3 - cp1);
    else
        file_name = fn;
}


output::pointer
output_filter_uuencode::create(const output::pointer &arg)
{
    return pointer(new output_filter_uuencode(arg));
}


void
output_filter_uuencode::write_inner(const void *p, size_t len)
{
    if (!begun)
    {
        begun = true;
        deeper_fputs("begin 644 ");
        deeper_fputs(file_name);
        deeper_fputc('\n');
    }
    const unsigned char *data = (const unsigned char *)p;
    if (len > 0)
        bol = (data[len - 1] == '\n');
    pos += len;
    while (len > 0)
    {
        unsigned char c = *data++;
        --len;
        residual_value = (residual_value << 8) | c;
        ++ipos;
        residual_bits += 8;
        for (;;)
        {
            residual_bits -= 6;
            int n = (residual_value >> residual_bits);
            n &= 0x3F;
            obuf[opos++] = etab[n];
            if (residual_bits == 0 && opos > 60)
            {
                obuf[0] = etab[ipos];
                obuf[opos++] = '\n';
                deeper_write(obuf, opos);
                opos = 1; // The length character
                ipos = 0;
            }
            if (residual_bits < 6)
                break;
        }
    }
}


nstring
output_filter_uuencode::filename(void)
    const
{
    return file_name;
}


long
output_filter_uuencode::ftell_inner(void)
    const
{
    return pos;
}


int
output_filter_uuencode::page_width(void)
    const
{
    return 80;
}


int
output_filter_uuencode::page_length(void)
    const
{
    return 66;
}


void
output_filter_uuencode::end_of_line_inner(void)
{
    if (!bol)
        fputc('\n');
}


nstring
output_filter_uuencode::type_name(void)
    const
{
    return ("uuencode " + output_filter::type_name());
}


// vim: set ts=8 sw=4 et :
