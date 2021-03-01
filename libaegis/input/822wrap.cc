//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2006 Peter Miller
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
// MANIFEST: functions to perfom rfc822 line wrapping
//

#include <common/trace.h>
#include <libaegis/input/822wrap.h>


input_822wrap::~input_822wrap()
{
    trace(("input_822wrap::~input_822wrap(this = %08lX)\n{\n", (long)this));
    pullback_transfer(deeper);
    trace(("}\n"));
}


input_822wrap::input_822wrap(input &arg) :
    deeper(arg),
    pos(0),
    column(0)
{
    trace(("input_822wrap::input_822wrap(this = %08lX)\n{\n", (long)this));
    trace(("}\n"));
}


long
input_822wrap::read_inner(void *data, size_t len)
{
    trace(("input_822wrap::read_inner(this = %08lX)\n{\n", (long)this));
    unsigned char *cp = (unsigned char *)data;
    unsigned char *end = cp + len;
    while (cp < end)
    {
	int c = deeper->getch();
	if (c < 0)
	    break;
	if (c == '\n' && column > 0)
	{
	    c = deeper->getch();
	    if (c != ' ' && c != '\t')
	    {
		if (c >= 0)
	    	    deeper->ungetc(c);
		c = '\n';
	    }
	}
	*cp++ = c;
	if (c == '\n')
	{
	    //
	    // Line buffered, because we don't want to get
	    // too far ahead of ourselves, in case the header
	    // finishes and the binary data starts.
	    //
	    column = 0;
	    break;
	}
	column++;
    }
    size_t nbytes = (cp - (unsigned char *)data);
    pos += nbytes;
    trace(("return %ld;\n", (long)nbytes));
    trace(("}\n"));
    return nbytes;
}


long
input_822wrap::ftell_inner()
{
    return pos;
}


nstring
input_822wrap::name()
{
    return deeper->name();
}


long
input_822wrap::length()
{
    return -1;
}


void
input_822wrap::keepalive()
{
    deeper->keepalive();
}


bool
input_822wrap::is_remote()
    const
{
    return deeper->is_remote();
}
