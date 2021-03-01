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
 * MANIFEST: functions to manipulate destructors
 */

#include <mem.h>
#include <str_list.h>


/*
 * NAME
 *	string_list_destructor - free a word list
 *
 * SYNOPSIS
 *	void string_list_destructor(string_list_ty *wlp);
 *
 * DESCRIPTION
 *	Wl_free is used to free the contents of a word list
 *	when it is finished with.
 *
 * CAVEAT
 *	It is assumed that the contents of the word list were all
 *	created using strdup() or similar, and grown using string_list_append().
 */

void
string_list_destructor(string_list_ty *wlp)
{
    size_t          j;

    for (j = 0; j < wlp->nstrings; j++)
	str_free(wlp->string[j]);
    if (wlp->string)
	mem_free(wlp->string);
    wlp->nstrings = 0;
    wlp->nstrings_max = 0;
    wlp->string = 0;
}
