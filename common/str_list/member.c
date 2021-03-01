/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate members
 */

#include <str_list.h>


/*
 * NAME
 *	string_list_member - word list membership
 *
 * SYNOPSIS
 *	int string_list_member(string_list_ty *wlp, string_ty *wp);
 *
 * DESCRIPTION
 *	Wl_member is used to determine if the given word is
 *	contained in the given word list.
 *
 * RETURNS
 *	A zero if the word is not in the list,
 *	and a non-zero if it is.
 */

int
string_list_member(string_list_ty *wlp, string_ty *w)
{
    size_t          j;

    for (j = 0; j < wlp->nstrings; j++)
	if (str_equal(wlp->string[j], w))
	    return 1;
    return 0;
}
