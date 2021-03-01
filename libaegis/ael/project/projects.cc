//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2004 Peter Miller;
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
// MANIFEST: functions to manipulate projectss
//

#include <ac/string.h>

#include <ael/change/inappropriat.h>
#include <ael/column_width.h>
#include <ael/project/inappropriat.h>
#include <ael/project/projects.h>
#include <col.h>
#include <option.h>
#include <output.h>
#include <project.h>
#include <project/history.h>
#include <str_list.h>
#include <trace.h>


void
list_projects(string_ty *project_name, long change_number, string_list_ty *args)
{
	string_list_ty	name;
	output_ty	*name_col = 0;
	output_ty	*dir_col = 0;
	output_ty	*desc_col = 0;
	size_t		j;
	int		left;
	col_ty		*colp;

	trace(("list_projects()\n{\n"));
	if (project_name)
		list_project_inappropriate();
	if (change_number)
		list_change_inappropriate();

	//
	// list the projects
	//
	project_list_get(&name);

	//
	// create the columns
	//
	colp = col_open((string_ty *)0);
	col_title(colp, "List of Projects", (char *)0);

	left = 0;
	name_col =
		col_create
		(
			colp,
			left,
			left + PROJECT_WIDTH,
			"Project\n---------"
		);
	left += PROJECT_WIDTH + 1;

	if (!option_terse_get())
	{
		dir_col =
			col_create
			(
				colp,
				left,
				left + DIRECTORY_WIDTH,
				"Directory\n-----------"
			);
		left += DIRECTORY_WIDTH + 1;

		desc_col =
			col_create
			(
				colp,
				left,
				0,
				"Description\n-------------"
			);
	}

	//
	// list each project
	//
	for (j = 0; j < name.nstrings; ++j)
	{
		project_ty	*pp;
		int		err;

		pp = project_alloc(name.string[j]);
		project_bind_existing(pp);

		err = project_is_readable(pp);

		output_put_str(name_col, project_name_get(pp));
		if (desc_col)
		{
			if (err)
				output_fputs(desc_col, strerror(err));
			else
			{
				string_ty	*top;

				//
				// The development directory of the
				// project change is the one which
				// contains the trunk or branch
				// baseline.
				//
				top = project_top_path_get(pp, 0);
				output_put_str(dir_col, top);

				output_put_str
				(
					desc_col,
					project_description_get(pp)
				);
			}
		}
		project_free(pp);
		col_eoln(colp);
	}

	//
	// clean up and go home
	//
	col_close(colp);
	trace(("}\n"));
}
