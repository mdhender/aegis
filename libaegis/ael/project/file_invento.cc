//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#include <libaegis/ael/column_width.h>
#include <libaegis/ael/project/file_invento.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <common/trace.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>


void
list_project_file_inventory(change_identifier &cid, string_list_ty *)
{
    trace(("list_project_file_inventory()\n{\n"));

    //
    // create the columns
    //
    col::pointer colp = col::open((string_ty *)0);
    string_ty *line1 = 0;
    if (cid.set())
    {
	line1 =
	    str_format
	    (
		"Project \"%s\"  Change %ld",
		project_name_get(cid.get_pp())->str_text,
		magic_zero_decode(cid.get_cp()->number)
	    );
    }
    else
    {
	line1 =
            str_format
            (
                "Project \"%s\"",
                project_name_get(cid.get_pp())->str_text
            );
    }
    colp->title(line1->str_text, "List of Project's File Inventory");
    str_free(line1);

    int left = 0;
    output::pointer file_name_col =
	colp->create(left, left + FILENAME_WIDTH, "File Name\n-----------");
    left += FILENAME_WIDTH + 1;
    output::pointer uuid_col =
	colp->create(left, left + UUID_WIDTH, "UUID\n------");
    left += UUID_WIDTH + 1;

    //
    // list the project's files
    //
    for (size_t j = 0;; ++j)
    {
	fstate_src_ty *src_data = cid.get_pp()->file_nth(j, view_path_extreme);
	if (!src_data)
	    break;
	if
        (
            cid.set()
        &&
            change_file_find(cid.get_cp(), src_data->file_name, view_path_first)
        )
	    continue;
	file_name_col->fputs(src_data->file_name);
	if (src_data->uuid)
	    uuid_col->fputs(src_data->uuid);
	else
	    uuid_col->fputs(src_data->file_name);
	colp->eoln();
    }
    trace(("}\n"));
}
