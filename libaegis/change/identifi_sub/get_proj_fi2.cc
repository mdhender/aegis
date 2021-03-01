//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: implementation of the change_identifi_sub_get_proj_fi2 class
//

#include <change/identifi_sub.h>
#include <project/file.h>
#include <project/file/roll_forward.h>


fstate_src_ty *
change_identifier_subset::get_project_file(const nstring &filename)
{
    if (need_historical_perspective())
    {
	project_file_roll_forward *hp = get_historian();
	file_event_ty *fep = hp->get_last(filename.get_ref());
	return (fep ? fep->src : 0);
    }
    return project_file_find(get_pp(), filename.get_ref(), view_path_simple);
}
