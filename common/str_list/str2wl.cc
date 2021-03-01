//
//	aegis - project change supervisor
//	Copyright (C) 2003-2006, 2008 Peter Miller
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
#include <common/ac/string.h>

#include <common/str_list.h>


void
string_list_ty::split(string_ty *s, const char *sep, bool ewhite)
{
    if (!sep)
    {
	sep = " \t\n\f\r";
	ewhite = true;
    }
    clear();
    const char *cp = s->str_text;
    bool more = true;
    while (*cp || more)
    {
	if (ewhite)
	{
	    while (isspace((unsigned char)*cp))
		cp++;
	}
	if (!*cp && !more)
	    break;
	more = false;
	const char *cp1 = cp;
	while (*cp && !strchr(sep, *cp))
	    cp++;
	const char *cp2 = cp;
	if (*cp)
	{
	    cp2 = cp + 1;
	    more = true;
	}
	if (ewhite)
	    while (cp > cp1 && isspace((unsigned char)cp[-1]))
		cp--;
	string_ty *w = str_n_from_c(cp1, cp - cp1);
	push_back(w);
	str_free(w);
	cp = cp2;
    }
}
