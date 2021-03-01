/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate catenates
 */

#include <ac/string.h>

#include <mem.h>
#include <str.h>


string_ty *
str_catenate(string_ty *s1, string_ty *s2)
{
    static char	*tmp;
    static size_t	tmplen;
    string_ty	*s;
    size_t		length;

    length = s1->str_length + s2->str_length;
    if (!tmp)
    {
	tmplen = length;
	if (tmplen < 16)
    	    tmplen = 16;
	tmp = mem_alloc(tmplen);
    }
    else
    {
	if (tmplen < length)
	{
    	    tmplen = length;
    	    tmp = mem_change_size(tmp, tmplen);
	}
    }
    memcpy(tmp, s1->str_text, s1->str_length);
    memcpy(tmp + s1->str_length, s2->str_text, s2->str_length);
    s = str_n_from_c(tmp, length);
    return s;
}
