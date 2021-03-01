//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate wl2strs
//

#include <ac/string.h>

#include <mem.h>
#include <str_list.h>


//
// NAME
//	wl2str - form a string from a word list
//
// SYNOPSIS
//	string_ty *wl2str(string_list_ty *wlp, int start, int stop, char *sep);
//
// DESCRIPTION
//	Wl2str is used to form a string from a word list.
//
// RETURNS
//	A pointer to the newly formed string in dynamic memory.
//
// CAVEAT
//	It is the responsibility of the caller to ensure that the
//	new string is freed when finished with, by a call to free().
//

string_ty      *
wl2str(const string_list_ty *wl, int start, int stop, const char *sep)
{
    int             j;
    static char     *tmp;
    static size_t   tmplen;
    size_t          length;
    size_t          seplen;
    char            *pos;
    string_ty       *s;

    if (!sep)
	sep = " ";
    seplen = strlen(sep);
    length = 0;
    for (j = start; j <= stop && j < (int)wl->nstrings; j++)
    {
	s = wl->string[j];
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

    pos = tmp;
    for (j = start; j <= stop && j < (int)wl->nstrings; j++)
    {
	s = wl->string[j];
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

    s = str_n_from_c(tmp, length);
    return s;
}
