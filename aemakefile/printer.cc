//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: implementation of the printer class
//

#pragma implementation "printer"

#include <nstring/list.h>
#include <printer.h>


printer::~printer()
{
}


printer::printer()
{
}


void
printer::puts(const char *s)
{
    while (*s)
	putch(*s++);
}


void
printer::write(const char *data, size_t len)
{
    while (len > 0)
    {
	putch(*data++);
	--len;
    }
}


printer &
operator<<(printer &p, const nstring_list &nsl)
{
    for (size_t j = 0; j < nsl.size(); ++j)
    {
	if (j)
	    p << ' ';
	p << nsl[j];
    }
    return p;
}
