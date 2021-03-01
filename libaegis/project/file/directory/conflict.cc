//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2006, 2008 Peter Miller
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

#include <libaegis/os/isa/path_prefix.h>
#include <libaegis/project/file.h>


string_ty *
project_file_directory_conflict(project_ty *pp, string_ty *file_name)
{
    size_t          j;
    fstate_src_ty   *src_data;

    for (j = 0; ; ++j)
    {
	//
        // Don't include deleted files in the check, so that directories
	// of removed files can re-used as file names.  This can
	// sometimes cause errors (unix will barf) when the user tries
	// to reconstruct later and they want mixed deltas in their
	// development directory.
	//
	// There used to be a larger problem where the history tool
	// could have a filename collision.  With the advent of the
	// UUIDs, this is a vanishing concern.
	//
	// include built files in the check
	//
	src_data = pp->file_nth(j, view_path_extreme);
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
