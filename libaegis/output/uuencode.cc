//
//	aegis - project change supervisor
//	Copyright (C) 2001-2006 Peter Miller
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
// MANIFEST: functions to manipulate uuencodes
//

#include <common/ac/string.h>

#include <libaegis/output/uuencode.h>
#include <common/str.h>

static char etab[] =
           "`!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";


output_uuencode_ty::~output_uuencode_ty()
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
	    deeper->write(obuf, opos);
	    ipos = 0;
	    opos = 1; // The length character
	}
	deeper->fputs("`\nend\n");
    }
    if (close_on_close)
	delete deeper;
    deeper = 0;
}


output_uuencode_ty::output_uuencode_ty(output_ty *arg1, bool arg2) :
    deeper(arg1),
    close_on_close(arg2),
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
    string_ty *fn = deeper->filename();
    const char *cp1 = fn->str_text;
    const char *cp2 = strrchr(cp1, '/');
    if (cp2)
	++cp2;
    else
	cp2 = cp1;
    const char *cp3 = strstr(cp2, ".uu");
    if (cp3)
	file_name = nstring(cp1, cp3 - cp1);
    else
	file_name = nstring(str_copy(fn));
}


void
output_uuencode_ty::write_inner(const void *p, size_t len)
{
    if (!begun)
    {
	begun = true;
	deeper->fputs("begin 644 ");
	deeper->fputs(file_name);
	deeper->fputc('\n');
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
		deeper->write(obuf, opos);
		opos = 1; // The length character
		ipos = 0;
	    }
	    if (residual_bits < 6)
		break;
	}
    }
}


string_ty *
output_uuencode_ty::filename()
    const
{
    return file_name.get_ref();
}


long
output_uuencode_ty::ftell_inner()
    const
{
    return pos;
}


int
output_uuencode_ty::page_width()
    const
{
    return 80;
}


int
output_uuencode_ty::page_length()
    const
{
    return 66;
}


void
output_uuencode_ty::end_of_line_inner()
{
    if (!bol)
	fputc('\n');
}


const char *
output_uuencode_ty::type_name()
    const
{
    return "uuencode";
}
