//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2006 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to encode output as MIME base 64
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


output_base64_ty::~output_base64_ty()
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
    if (delete_deeper)
	delete deeper;
    deeper = 0;
}


output_base64_ty::output_base64_ty(output_ty *arg1, bool arg2) :
    deeper(arg1),
    delete_deeper(arg2),
    residual_value(0),
    residual_bits(0),
    output_column(0),
    pos(0),
    bol(true)
{
}


void
output_base64_ty::write_inner(const void *p, size_t len)
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


string_ty *
output_base64_ty::filename()
    const
{
    return deeper->filename();
}


long
output_base64_ty::ftell_inner()
    const
{
    return pos;
}


int
output_base64_ty::page_width()
    const
{
    return deeper->page_width();
}


int
output_base64_ty::page_length()
    const
{
    return deeper->page_length();
}


void
output_base64_ty::end_of_line_inner()
{
    if (!bol)
	write_inner("\n", 1);
}


const char *
output_base64_ty::type_name()
    const
{
    return "base64";
}
