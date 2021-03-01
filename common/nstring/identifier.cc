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
// MANIFEST: implementation of the nstring::identifier method
//

#include <ac/ctype.h>

#include <nstring.h>
#include <nstring/accumulator.h>


nstring
nstring::identifier()
    const
{
    static nstring_accumulator buf;
    buf.clear();
    const char *cp = c_str();
    for (;;)
    {
	unsigned char c = *cp++;
	if (!c)
		break;
	// C locale
	if (!isalnum((unsigned char)c))
    	    c = '_';
	buf.push_back(c);
    }
    return buf.mkstr();
}
