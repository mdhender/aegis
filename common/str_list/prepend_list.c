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
 * MANIFEST: functions to manipulate prepend_lists
 */

#include <mem.h>
#include <str_list.h>


void
string_list_prepend_list(string_list_ty *wlp, string_list_ty  *arg)
{
    size_t          j;

    if (wlp->nstrings + arg->nstrings > wlp->nstrings_max)
    {
	size_t          nbytes;

	/*
	 * always 8 less than a power of 2, which is
	 * most efficient for many memory allocators
	 */
	for (;;)
	{
	    wlp->nstrings_max = wlp->nstrings_max * 2 + 8;
	    if (wlp->nstrings + arg->nstrings <= wlp->nstrings_max)
		break;
	}
	nbytes = wlp->nstrings_max * sizeof(string_ty *);
	wlp->string = (string_ty **)mem_change_size(wlp->string, nbytes);
    }
    for (j = wlp->nstrings; j > 0; --j)
	wlp->string[j + arg->nstrings - 1] = wlp->string[j - 1];
    for (j = 0; j < arg->nstrings; ++j)
	wlp->string[j] = str_copy(arg->string[j]);
    wlp->nstrings += arg->nstrings;
}
