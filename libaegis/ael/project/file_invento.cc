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
// MANIFEST: implementation of the ael_project_file_invento class
//

#include <ael/column_width.h>
#include <ael/project/file_invento.h>
#include <change.h>
#include <change/file.h>
#include <col.h>
#include <project.h>
#include <project/file.h>
#include <trace.h>
#include <user.h>
#include <zero.h>


void
list_project_file_inventory(string_ty *project_name, long change_number,
    string_list_ty *args)
{
    //
    // locate project data
    //
    trace(("list_project_file_inventory()\n{\n"));
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
    change_ty *cp = 0;
    if (change_number)
    {
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
    }

    //
    // create the columns
    //
    col_ty *colp = col_open((string_ty *)0);
    string_ty *line1 = 0;
    if (change_number)
    {
	line1 =
	    str_format
	    (
		"Project \"%s\"  Change %ld",
		project_name_get(pp)->str_text,
		magic_zero_decode(change_number)
	    );
    }
    else
	line1 = str_format("Project \"%s\"", project_name_get(pp)->str_text);
    col_title(colp, line1->str_text, "List of Project's File Inventory");
    str_free(line1);

    int left = 0;
    output_ty *file_name_col =
	col_create(colp, left, left + FILENAME_WIDTH, "File Name\n-----------");
    left += FILENAME_WIDTH + 1;
    output_ty *uuid_col =
	col_create(colp, left, left + UUID_WIDTH, "UUID\n------");
    left += UUID_WIDTH + 1;

    //
    // list the project's files
    //
    for (size_t j = 0;; ++j)
    {
	fstate_src_ty *src_data = project_file_nth(pp, j, view_path_extreme);
	if (!src_data)
	    break;
	if (cp && change_file_find(cp, src_data->file_name, view_path_first))
	    continue;
	file_name_col->fputs(src_data->file_name);
	if (src_data->uuid)
	    uuid_col->fputs(src_data->uuid);
	else
	    uuid_col->fputs(src_data->file_name);
	col_eoln(colp);
    }

    //
    // clean up and go home
    //
    col_close(colp);
    project_free(pp);
    if (cp)
	change_free(cp);
    user_free(up);
    trace(("}\n"));
}
