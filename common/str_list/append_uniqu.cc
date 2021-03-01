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
// MANIFEST: functions to manipulate append_uniqus
//

#include <str_list.h>


//
// NAME
//	wl_insert - a insert a word into a list
//
// SYNOPSIS
//	void wl_insert(string_list_ty *wlp, string_ty *wp);
//
// DESCRIPTION
//	Wl_insert is similar to string_list_append, however it does not
//	append the word unless it is not already in the list.
//
// CAVEAT
//	If the word is inserted it is copied.
//

void
string_list_append_unique(string_list_ty *wlp, string_ty *wp)
{
    size_t          j;

    for (j = 0; j < wlp->nstrings; j++)
	if (str_equal(wlp->string[j], wp))
	    return;
    string_list_append(wlp, wp);
}
