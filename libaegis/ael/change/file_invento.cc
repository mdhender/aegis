//
//	aegis - project change supervisor
//	Copyright (C) 2004-2007 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/ael/change/file_invento.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/col.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <common/trace.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>


void
list_change_file_inventory(string_ty *project_name, long change_number,
    string_list_ty *)
{
    //
    // locate project data
    //
    trace(("list_change_files()\n{\n"));
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
    else
	project_name = str_copy(project_name);
    project_ty *pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    user_ty::pointer up = user_ty::create();

    //
    // locate change data
    //
    if (!change_number)
	change_number = up->default_change(pp);
    change::pointer cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // create the columns
    //
    col *colp = col::open((string_ty *)0);
    string_ty *line1 =
	str_format
	(
	    "Project \"%s\"  Change %ld",
	    project_name_get(pp)->str_text,
	    magic_zero_decode(change_number)
	);
    colp->title(line1->str_text, "List of Change's File Inventory");
    str_free(line1);

    int left = 0;
    output_ty *file_name_col =
	colp->create(left, left + FILENAME_WIDTH, "File Name\n-----------");
    left += FILENAME_WIDTH + 1;
    output_ty *uuid_col =
	colp->create(left, left + UUID_WIDTH, "UUID\n------");
    left += UUID_WIDTH + 1;

    //
    // list the change's files
    //
    bool indev = !cp->is_completed() && !change_was_a_branch(cp);
    for (size_t j = 0;; ++j)
    {
	fstate_src_ty *src_data = change_file_nth(cp, j, view_path_first);
	if (!src_data)
	    break;
	file_name_col->fputs(src_data->file_name);
	if (src_data->uuid)
	    uuid_col->fputs(src_data->uuid);
	else if (indev && src_data->action == file_action_create)
	    uuid_col->fputs("# uuid to be set by integrate pass");
	else
	    uuid_col->fputs(src_data->file_name);
	colp->eoln();
    }

    //
    // clean up and go home
    //
    delete colp;
    project_free(pp);
    change_free(cp);
    trace(("}\n"));
}
