//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/change/identifi_sub.h>
#include <libaegis/file/event.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>


fstate_src_ty *
change_identifier_subset::get_project_file(const nstring &filename)
{
    if (need_historical_perspective())
    {
	project_file_roll_forward *hp = get_historian();
	file_event *fep = hp->get_last(filename.get_ref());
	return (fep ? fep->get_src() : 0);
    }
    return project_file_find(get_pp(), filename.get_ref(), view_path_simple);
}
