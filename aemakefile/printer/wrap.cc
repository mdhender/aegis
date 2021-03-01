//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <common/ac/string.h>

#include <aemakefile/printer/wrap.h>


printer_wrap::~printer_wrap()
{
    if (column != 0)
	putch('\n');
    delete deeper;
    deeper = 0;
    buflen = 0;
    bufmax = 0;
    delete [] buffer;
    buffer = 0;
}


printer_wrap::printer_wrap(printer *arg) :
    deeper(arg),
    start_of_line(true),
    continuation_line(false),
    column(0),
    buflen(0),
    bufwidth(0),
    bufmax(0),
    buffer(0)
{
}


void
printer_wrap::stash_inner(char c)
{
    if (buflen >= bufmax)
    {
	size_t new_bufmax = bufmax * 2 + 32;
	char *new_buffer = new char[new_bufmax];
	memcpy(new_buffer, buffer, buflen);
	delete [] buffer;
	buffer = new_buffer;
	bufmax = new_bufmax;
    }
    buffer[buflen++] = c;
    if (c == '\t')
	bufwidth += 8;
    else
	bufwidth++;
}


void
printer_wrap::emit_buffer()
{
    if (!buflen)
	return;
    //
    // We have to decide if the word in the buffer makes the line wrap
    // or not.  The current output position in the deeper output in
    // represented by "column", and the length of the current work is
    // represented by "buflen".  If we need to wrap, we add " \\\n\t\t",
    // so we have to wrap at column 78 or earlier.
    //
    if (column != 0)
    {
	if (column + buflen > maximum_line_width)
	{
	    deeper->puts(" \\\n\t\t");
	    column = 16;
	    continuation_line = true;
	}
	else
	{
	    deeper->putch(' ');
	    ++column;
	}
    }
    column += bufwidth;
    deeper->write(buffer, buflen);
    start_of_line = false;
    buflen = 0;
    bufwidth = 0;
}


void
printer_wrap::putch(char c)
{
    switch (c)
    {
    case '\n':
	emit_buffer();

	deeper->putch('\n');
	column = 0;
	start_of_line = true;
	continuation_line = false;
	break;

    case ' ':
    case '\t':
	if (start_of_line && !continuation_line)
	    stash_inner(c);
	else
	    emit_buffer();
	break;

    default:
	stash_inner(c);
	start_of_line = false;
	break;
    }
}
