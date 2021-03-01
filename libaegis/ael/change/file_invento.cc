//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: implementation of the ael_change_file_invento class
//

#include <ael/change/file_invento.h>
#include <ael/column_width.h>
#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <col.h>
#include <output.h>
#include <project.h>
#include <trace.h>
#include <user.h>
#include <zero.h>


void
list_change_file_inventory(string_ty *project_name, long change_number,
    string_list_ty *args)
{
    //
    // locate project data
    //
    trace(("list_change_files()\n{\n"));
    if (!project_name)
	project_name = user_default_project();
    else
	project_name = str_copy(project_name);
    project_ty *pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    //
    // locate user data
    //
    user_ty *up = user_executing(pp);

    //
    // locate change data
    //
    if (!change_number)
	change_number = user_default_change(up);
    change_ty *cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // create the columns
    //
    col_ty *colp = col_open((string_ty *)0);
    string_ty *line1 =
	str_format
	(
	    "Project \"%s\"  Change %ld",
	    project_name_get(pp)->str_text,
	    magic_zero_decode(change_number)
	);
    col_title(colp, line1->str_text, "List of Change's File Inventory");
    str_free(line1);

    int left = 0;
    output_ty *file_name_col =
	col_create(colp, left, left + FILENAME_WIDTH, "File Name\n-----------");
    left += FILENAME_WIDTH + 1;
    output_ty *uuid_col =
	col_create(colp, left, left + UUID_WIDTH, "UUID\n------");
    left += UUID_WIDTH + 1;

    //
    // list the change's files
    //
    bool indev = !change_is_completed(cp) && !change_was_a_branch(cp);
    for (size_t j = 0;; ++j)
    {
	fstate_src_ty *src_data = change_file_nth(cp, j, view_path_first);
	if (!src_data)
	    break;
	output_put_str(file_name_col, src_data->file_name);
	if (src_data->uuid)
	    output_put_str(uuid_col, src_data->uuid);
	else if (indev && src_data->action == file_action_create)
	    output_fputs(uuid_col, "# uuid to be set by integrate pass");
	else
	    output_put_str(uuid_col, src_data->file_name);
	col_eoln(colp);
    }

    //
    // clean up and go home
    //
    col_close(colp);
    project_free(pp);
    change_free(cp);
    user_free(up);
    trace(("}\n"));
}
