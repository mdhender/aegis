//
//	aegis - project change supervisor
//	Copyright (C) 2001-2005 Peter Miller;
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
// MANIFEST: functions to manipulate downcases
//

#include <common/ac/ctype.h>

#include <common/mem.h>
#include <common/str.h>


string_ty *
str_downcase(string_ty *s)
{
    static char     *tmp;
    static size_t   tmplen;
    string_ty	    *retval;
    char	    *cp1;
    char	    *cp2;

    if (s->str_length > tmplen)
    {
	for (;;)
	{
	    tmplen = tmplen * 2 + 8;
	    if (s->str_length <= tmplen)
		break;
	}
	tmp = (char *)mem_change_size(tmp, tmplen);
    }
    for (cp1 = s->str_text, cp2 = tmp; *cp1; ++cp1, ++cp2)
    {
	unsigned char c = *cp1;
	if (isupper((unsigned char)c))
    	    c = tolower(c);
	*cp2 = c;
    }
    retval = str_n_from_c(tmp, s->str_length);
    return retval;
}
