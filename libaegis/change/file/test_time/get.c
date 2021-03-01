/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate gets
 */

#include <change/file.h>


time_t
change_file_test_time_get(change_ty *cp, fstate_src src_data,
    string_ty *variant)
{
    fstate_src_architecture_times_list atlp;
    fstate_src_architecture_times atp;
    size_t          j;

    atlp = src_data->architecture_times;
    if (!atlp)
	return 0;
    if (!variant)
	variant = change_architecture_name(cp, 1);
    for (j = 0; j < atlp->length; ++j)
    {
	atp = atlp->list[j];
	if
	(
	    /* bug if not set */
	    atp->variant
	&&
	    str_equal(atp->variant, variant)
	)
	    return atp->test_time;
    }
    return 0;
}
