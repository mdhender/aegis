//
//	aegis - project change supervisor
//	Copyright (C) 2003-2005 Peter Miller;
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
// MANIFEST: method to unsplit string lists
//

#include <common/ac/string.h>

#include <common/mem.h>
#include <common/str_list.h>


string_ty *
string_list_ty::unsplit(const char *sep)
    const
{
    return unsplit(0, size(), sep);
}


string_ty *
string_list_ty::unsplit(size_t start, size_t stop, const char *sep)
    const
{
    static char *tmp;
    static size_t tmplen;

    if (!sep)
	sep = " ";
    size_t seplen = strlen(sep);
    size_t length = 0;
    size_t j;
    for (j = start; j <= stop && j < nstrings; j++)
    {
	string_ty *s = string[j];
	if (s->str_length)
	{
	    if (length)
		length += seplen;
	    length += s->str_length;
	}
    }

    if (tmplen < length)
    {
	tmplen = length;
	tmp = (char *)mem_change_size(tmp, tmplen);
    }

    char *pos = tmp;
    for (j = start; j <= stop && j < nstrings; j++)
    {
	string_ty *s = string[j];
	if (s->str_length)
	{
	    if (pos != tmp)
	    {
		memcpy(pos, sep, seplen);
		pos += seplen;
	    }
	    memcpy(pos, s->str_text, s->str_length);
	    pos += s->str_length;
	}
    }

    return str_n_from_c(tmp, length);
}
