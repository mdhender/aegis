//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2006 Peter Miller
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
// MANIFEST: functions to manipulate prefixs
//

#include <libaegis/output/prefix.h>


output_prefix_ty::~output_prefix_ty()
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


output_prefix_ty::output_prefix_ty(output_ty *arg1, bool arg2,
       	const char *arg3) :
    deeper(arg1),
    close_on_close(arg2),
    prefix(arg3),
    column(0),
    pos(0)
{
}


void
output_prefix_ty::write_inner(const void *p, size_t len)
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
output_prefix_ty::flush_inner()
{
    deeper->flush();
}


string_ty *
output_prefix_ty::filename()
    const
{
    return deeper->filename();
}


long
output_prefix_ty::ftell_inner()
    const
{
    return pos;
}


int
output_prefix_ty::page_width()
    const
{
    return deeper->page_width();
}


int
output_prefix_ty::page_length()
    const
{
    return deeper->page_length();
}


void
output_prefix_ty::end_of_line_inner()
{
    if (column != 0)
	fputc('\n');
}


const char *
output_prefix_ty::type_name()
    const
{
    return "prefix";
}
