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
// MANIFEST: functions to manipulate copys
//

#include <str_list.h>


//
// NAME
//	string_list_copy - copy a word list
//
// SYNOPSIS
//	void string_list_copy(string_list_ty *to, string_list_ty *from);
//
// DESCRIPTION
//	Wl_copy is used to copy word lists.
//
// RETURNS
//	A copy of the 'to' word list is placed in 'from'.
//
// CAVEAT
//	It is the responsibility of the caller to ensure that the
//	new word list is freed when finished with, by a call to
//	string_list_destructor().
//

void
string_list_copy(string_list_ty *to, const string_list_ty *from)
{
    size_t          j;

    string_list_constructor(to);
    for (j = 0; j < from->nstrings; j++)
	string_list_append(to, str_copy(from->string[j]));
}
