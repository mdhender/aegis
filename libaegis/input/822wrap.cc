//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2003-2006, 2008, 2012 Peter Miller
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

#include <common/trace.h>
#include <libaegis/input/822wrap.h>


input_822wrap::~input_822wrap()
{
    trace(("input_822wrap::~input_822wrap(this = %p)\n{\n", this));
    pullback_transfer(deeper);
    trace(("}\n"));
}


input_822wrap::input_822wrap(input &arg) :
    deeper(arg),
    pos(0),
    column(0)
{
    trace(("input_822wrap::input_822wrap(this = %p)\n{\n", this));
    trace(("}\n"));
}


ssize_t
input_822wrap::read_inner(void *data, size_t len)
{
    trace(("input_822wrap::read_inner(this = %p)\n{\n", this));
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


off_t
input_822wrap::ftell_inner()
{
    return pos;
}


nstring
input_822wrap::name()
{
    return deeper->name();
}


off_t
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


// vim: set ts=8 sw=4 et :
