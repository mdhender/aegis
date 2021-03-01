//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2008 Peter Miller
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

#include <libaegis/change/file.h>
#include <libaegis/os/isa/path_prefix.h>


string_ty *
change_file_directory_conflict(change::pointer cp, string_ty *file_name)
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
