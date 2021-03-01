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
 * MANIFEST: functions to manipulate subsets
 */

#include <str_list.h>


int
string_list_subset(string_list_ty *a, string_list_ty *b)
{
    size_t          j, k;

    /*
     * test if "a is a subset of b"
     */
    if (a->nstrings > b->nstrings)
	return 0;
    for (j = 0; j < a->nstrings; ++j)
    {
	for (k = 0; k < b->nstrings; ++k)
	    if (str_equal(a->string[j], b->string[k]))
		break;
	if (k >= b->nstrings)
	    return 0;
    }
    return 1;
}
