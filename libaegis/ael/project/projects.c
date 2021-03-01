/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate projectss
 */

#include <ac/string.h>

#include <ael/change/inappropriat.h>
#include <ael/column_width.h>
#include <ael/project/inappropriat.h>
#include <ael/project/projects.h>
#include <col.h>
#include <option.h>
#include <project.h>
#include <project_hist.h>
#include <str_list.h>
#include <trace.h>


void
list_projects(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	string_list_ty	name;
	int		name_col = 0;
	int		dir_col = 0;
	int		desc_col = 0;
	int		j;
	int		left;

	trace(("list_projects()\n{\n"));
	if (project_name)
		list_project_inappropriate();
	if (change_number)
		list_change_inappropriate();

	/*
	 * list the projects
	 */
	project_list_get(&name);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	col_title("List of Projects", (char *)0);

	left = 0;
	name_col = col_create(left, left + PROJECT_WIDTH);
	left += PROJECT_WIDTH + 1;
	col_heading(name_col, "Project\n---------");

	if (!option_terse_get())
	{
		dir_col = col_create(left, left + DIRECTORY_WIDTH);
		left += DIRECTORY_WIDTH + 1;
		col_heading(dir_col, "Directory\n-----------");

		desc_col = col_create(left, 0);
		col_heading(desc_col, "Description\n-------------");
	}

	/*
	 * list each project
	 */
	for (j = 0; j < name.nstrings; ++j)
	{
		project_ty	*pp;
		int		err;

		pp = project_alloc(name.string[j]);
		project_bind_existing(pp);

		err = project_is_readable(pp);

		col_puts(name_col, project_name_get(pp)->str_text);
		if (!option_terse_get())
		{
			if (err)
				col_puts(desc_col, strerror(err));
			else
			{
				string_ty	*top;

				/*
				 * The development directory of the
				 * project change is the one which
				 * contains the trunk or branch
				 * baseline.
				 */
				top = project_top_path_get(pp, 0);
				col_puts(dir_col, top->str_text);

				col_puts
				(
					desc_col,
					project_description_get(pp)->str_text
				);
			}
		}
		project_free(pp);
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	trace(("}\n"));
}
