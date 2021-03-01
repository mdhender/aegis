//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: implementation of the html_url_unquote class
//

#include <common/nstring.h>
#include <common/nstring/accumulator.h>


static int
unhex(int c)
{
    switch (c)
    {
    default:
	return -1;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	return (c - '0');

    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
	return (c - 'a' + 10);

    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
	return (c - 'A' + 10);
    }
}


nstring
nstring::url_unquote()
    const
{
    static nstring_accumulator sa;
    sa.clear();
    const char *sp = c_str();
    for (;;)
    {
	unsigned char c = *sp++;
	if (!c)
	    break;
	int n1, n2;
	if (c == '%' && (n1 = unhex(sp[0])) >= 0 && (n2 = unhex(sp[1])) >= 0)
	{
	    c = (n1 << 4) + n2;
	    sp += 2;
	}
	sa.push_back(c);
    }
    return sa.mkstr();
}
