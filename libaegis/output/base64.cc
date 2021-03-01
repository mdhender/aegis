//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/output/base64.h>
#include <common/str.h>

static char base64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline int
map64(int n)
{
    return base64[n & 0x3F];
}


output_base64::~output_base64()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    switch (residual_bits)
    {
    case 4:
	deeper->fputc(map64(residual_value << 2));
	deeper->fputc('=');
	output_column += 2;
	break;

    case 2:
	deeper->fputc(map64(residual_value << 4));
	deeper->fputc('=');
	deeper->fputc('=');
	output_column += 3;
	break;
    }
    if (output_column)
	deeper->fputc('\n');
}


output_base64::output_base64(const output::pointer &a_deeper) :
    deeper(a_deeper),
    residual_value(0),
    residual_bits(0),
    output_column(0),
    pos(0),
    bol(true)
{
}


output::pointer
output_base64::create(const output::pointer &a_deeper)
{
    return pointer(new output_base64(a_deeper));
}


void
output_base64::write_inner(const void *p, size_t len)
{
    const unsigned char *data = (const unsigned char *)p;
    while (len > 0)
    {
	unsigned char c = *data++;
	--len;

	residual_value = (residual_value << 8) | c;
	residual_bits += 8;
	for (;;)
	{
	    residual_bits -= 6;
	    deeper->fputc(map64(residual_value >> residual_bits));
	    ++output_column;
	    if (residual_bits == 0 && output_column > 70)
	    {
		deeper->fputc('\n');
		output_column = 0;
	    }
	    if (residual_bits < 6)
		break;
	}
	++pos;
	bol = (c == '\n');
    }
}


nstring
output_base64::filename()
    const
{
    return deeper->filename();
}


long
output_base64::ftell_inner()
    const
{
    return pos;
}


int
output_base64::page_width()
    const
{
    return deeper->page_width();
}


int
output_base64::page_length()
    const
{
    return deeper->page_length();
}


void
output_base64::end_of_line_inner()
{
    if (!bol)
	write_inner("\n", 1);
}


const char *
output_base64::type_name()
    const
{
    return "base64";
}
