//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: functions to manipulate uuids
//

#include <libaegis/change/file.h>


fstate_src_ty *
change_file_find_uuid(change_ty *cp, string_ty *uuid, view_path_ty view_path)
{
    size_t          j;

    for (j = 0; ; ++j)
    {
	fstate_src_ty   *src;

	src = change_file_nth(cp, j, view_path);
	if (!src)
	    return 0;
	if (src->uuid && str_equal(uuid, src->uuid))
	{
            //
            // When a file is renamed, it appears in the change's file
            // list twice: once for the remove and again for the create.
            // Both entires have the same UUID.  Most of the time the
            // user is expecting the create side (with the new name) not
            // the remove side.
	    //
	    if (src->action != file_action_remove || !src->move)
		return src;
	}
    }
}
