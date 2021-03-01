/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate quotes
 */

#include <str_list.h>


void
string_list_quote_shell(string_list_ty *to, string_list_ty *from)
{
    size_t		j;
    string_ty	*s;

    string_list_constructor(to);
    for (j = 0; j < from->nstrings; ++j)
    {
	s = str_quote_shell(from->string[j]);
	string_list_append(to, s);
	str_free(s);
    }
}
