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

#include <common/ac/string.h>

#include <common/str.h>


string_ty *
str_field(string_ty *s, int sep, int fldnum)
{
    char	    *cp;
    char	    *ep;

    cp = s->str_text;
    while (fldnum > 0)
    {
	ep = strchr(cp, sep);
	if (!ep)
	    return str_from_c("");
	cp = ep + 1;
	--fldnum;
    }
    ep = strchr(cp, sep);
    if (ep)
	return str_n_from_c(cp, ep - cp);
    return str_from_c(cp);
}
