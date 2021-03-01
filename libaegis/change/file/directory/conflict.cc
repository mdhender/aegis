//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate conflicts
//

#include <change/file.h>
#include <os/isa/path_prefix.h>


string_ty *
change_file_directory_conflict(change_ty *cp, string_ty *file_name)
{
	size_t		j;
	fstate_src_ty   *src_data;

	for (j = 0; ; ++j)
	{
		//
		// include deleted files in the check,
		// so we can reconstruct later
		//
		// include built files in the check
		//
		src_data = change_file_nth(cp, j, view_path_first);
		if (!src_data)
			break;
		if
		(
			os_isa_path_prefix(file_name, src_data->file_name)
		||
			os_isa_path_prefix(src_data->file_name, file_name)
		)
			return src_data->file_name;
	}
	return 0;
}