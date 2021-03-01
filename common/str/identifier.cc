//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller
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
// MANIFEST: functions to manipulate identifiers
//

#include <common/ac/ctype.h>

#include <common/str.h>
#include <common/stracc.h>


string_ty *
str_identifier(string_ty *s)
{
    static stracc_t ac;
    ac.clear();
    for (const char *cp = s->str_text; ; ++cp)
    {
	unsigned char c = *cp;
	if (!c)
	    break;
	// C locale
	if (!isalnum(c))
    	    c = '_';
	ac.push_back(c);
    }
    return ac.mkstr();
}
