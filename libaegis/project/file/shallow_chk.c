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
 * MANIFEST: functions to manipulate shallow_chks
 */

#include <change.h>
#include <change/file.h>
#include <project/file.h>


int
project_file_shallow_check(project_ty *pp, string_ty *file_name)
{
    change_ty	*pcp;
    fstate_src	src_data;

    if (!pp->parent)
    {
	/* accelerator */
	return 1;
    }
    pcp = project_change_get(pp);
    src_data = change_file_find(pcp, file_name);
    return (src_data != 0);
}
