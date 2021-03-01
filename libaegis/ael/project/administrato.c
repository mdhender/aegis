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
 * MANIFEST: functions to manipulate administratos
 */

#include <ael/column_width.h>
#include <ael/change/inappropriat.h>
#include <ael/project/administrato.h>
#include <col.h>
#include <option.h>
#include <project.h>
#include <project_hist.h>
#include <trace.h>
#include <user.h>


void
list_administrators(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	project_ty	*pp;
	int		login_col = 0;
	int		name_col = 0;
	int		j;
	string_ty	*line1;
	int		left;

	trace(("list_administrators()\n{\n"));
	if (change_number)
		list_change_inappropriate();

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 = str_format("Project \"%S\"", project_name_get(pp));
	col_title(line1->str_text, "List of Administrators");
	str_free(line1);

	left = 0;
	login_col = col_create(left, left + LOGIN_WIDTH);
	left += LOGIN_WIDTH + 2;
	col_heading(login_col, "User\n------");

	if (!option_terse_get())
	{
		name_col = col_create(left, 0);
		col_heading(name_col, "Full Name\n-----------");
	}

	/*
	 * list project's administrators
	 */
	for (j = 0; ; ++j)
	{
		string_ty	*logname;

		logname = project_administrator_nth(pp, j);
		if (!logname)
			break;
		col_puts(login_col, logname->str_text);
		if (!option_terse_get())
			col_puts(name_col, user_full_name(logname));
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	project_free(pp);
	trace(("}\n"));
}
