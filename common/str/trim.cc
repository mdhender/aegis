//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004-2006, 2008 Peter Miller
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

#include <common/ac/ctype.h>

#include <common/language.h>
#include <common/str.h>
#include <common/stracc.h>


string_ty *
str_trim(string_ty *s)
{
    int		    whitespace;
    static stracc_t buffer;
    char	    *cp;
    string_ty	    *result;

    whitespace = 0;
    buffer.clear();
    language_human();
    for (cp = s->str_text; *cp; ++cp)
    {
	if (isspace((unsigned char)*cp))
	    ++whitespace;
	else
	{
	    if (whitespace)
	    {
	       	if (buffer.size())
		    buffer.push_back(' ');
	       	whitespace = 0;
	    }
	    buffer.push_back(*cp);
	}
    }
    language_C();
    result = buffer.mkstr();
    return result;
}
