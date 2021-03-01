//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2006, 2008 Peter Miller
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

#include <libaegis/output/prefix.h>


output_prefix::~output_prefix()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();
}


output_prefix::output_prefix(const output::pointer &a_deeper,
       	const char *a_prefix) :
    deeper(a_deeper),
    prefix(a_prefix),
    column(0),
    pos(0)
{
}


output::pointer
output_prefix::create(const output::pointer &a_deeper, const char *a_prefix)
{
    return pointer(new output_prefix(a_deeper, a_prefix));
}


void
output_prefix::write_inner(const void *p, size_t len)
{
    const unsigned char *data = (unsigned char *)p;
    const unsigned char *begin = data;
    while (len > 0)
    {
	unsigned char c = *data++;
	--len;

	if (column == 0)
	{
	    deeper->fputs(prefix);
	    pos += prefix.length();
	}
	if (c == '\n')
	{
	    size_t nbytes = data - begin;
	    deeper->write(begin, nbytes);
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
	deeper->write(begin, nbytes);
	pos += nbytes;
    }
}


void
output_prefix::flush_inner()
{
    deeper->flush();
}


nstring
output_prefix::filename()
    const
{
    return deeper->filename();
}


long
output_prefix::ftell_inner()
    const
{
    return pos;
}


int
output_prefix::page_width()
    const
{
    return deeper->page_width();
}


int
output_prefix::page_length()
    const
{
    return deeper->page_length();
}


void
output_prefix::end_of_line_inner()
{
    if (column != 0)
	fputc('\n');
}


const char *
output_prefix::type_name()
    const
{
    return "prefix";
}
