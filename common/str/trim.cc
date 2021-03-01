//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate trims
//

#include <ac/ctype.h>

#include <language.h>
#include <str.h>
#include <stracc.h>


string_ty *
str_trim(string_ty *s)
{
    int		    whitespace;
    static stracc_t buffer;
    char	    *cp;
    string_ty	    *result;

    whitespace = 0;
    stracc_open(&buffer);
    language_human();
    for (cp = s->str_text; *cp; ++cp)
    {
	if (isspace((unsigned char)*cp))
	    ++whitespace;
	else
	{
	    if (whitespace)
	    {
	       	if (buffer.length)
		    stracc_char(&buffer, ' ');
	       	whitespace = 0;
	    }
	    stracc_char(&buffer, *cp);
	}
    }
    language_C();
    result = stracc_close(&buffer);
    return result;
}
