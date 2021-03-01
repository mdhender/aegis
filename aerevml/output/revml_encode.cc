//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
//	All rights reserved.
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
// MANIFEST: implementation of the output_revml_encode class
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

    if (close_on_close)
	delete deeper;
    deeper = 0;
}


output_revml_encode::output_revml_encode(output_ty *arg1, bool arg2) :
    deeper(arg1),
    close_on_close(arg2),
    pos(0),
    bol(true)
{
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
output_revml_encode::flush_inner()
{
    deeper->flush();
}


string_ty *
output_revml_encode::filename()
    const
{
    return deeper->filename();
}


long
output_revml_encode::ftell_inner()
    const
{
    return pos;
}


int
output_revml_encode::page_width()
    const
{
    return deeper->page_width();
}


int
output_revml_encode::page_length()
    const
{
    return deeper->page_length();
}


void
output_revml_encode::end_of_line_inner()
{
    if (!bol)
	fputc('\n');
}


const char *
output_revml_encode::type_name()
    const
{
    return "revml";
}
