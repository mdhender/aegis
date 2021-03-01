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
// MANIFEST: functions to manipulate removes
//

#include <str_list.h>


//
// NAME
//	string_list_remove - remove list member
//
// SYNOPSIS
//	void string_list_remove(string_list_ty *wlp, string_ty *wp);
//
// DESCRIPTION
//	The string_list_remove function is used to delete a member of
//	a word list.
//
// RETURNS
//	void
//

void
string_list_remove(string_list_ty *wlp, string_ty *wp)
{
    size_t          j;
    size_t          k;

    for (j = 0; j < wlp->nstrings; ++j)
    {
	if (str_equal(wlp->string[j], wp))
	{
	    wlp->nstrings--;
	    for (k = j; k < wlp->nstrings; ++k)
		wlp->string[k] = wlp->string[k + 1];
	    str_free(wp);
	    break;
	}
    }
}
