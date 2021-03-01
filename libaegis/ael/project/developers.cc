//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2005 Peter Miller;
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
// MANIFEST: functions to manipulate developerss
//

#include <ael/change/inappropriat.h>
#include <ael/column_width.h>
#include <ael/project/developers.h>
#include <col.h>
#include <option.h>
#include <output.h>
#include <project.h>
#include <project/history.h>
#include <str_list.h>
#include <trace.h>
#include <user.h>


void
list_developers(string_ty *project_name, long change_number,
    string_list_ty *args)
{
    project_ty      *pp;
    output_ty       *login_col = 0;
    output_ty       *name_col = 0;
    int             j;
    string_ty       *line1;
    int             left;
    col_ty          *colp;

    trace(("list_developers()\n{\n"));
    if (change_number)
	list_change_inappropriate();

    //
    // locate project data
    //
    if (!project_name)
	project_name = user_default_project();
    else
	project_name = str_copy(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    //
    // create the columns
    //
    colp = col_open((string_ty *)0);
    line1 = str_format("Project \"%s\"", project_name_get(pp)->str_text);
    col_title(colp, line1->str_text, "List of Developers");
    str_free(line1);

    left = 0;
    login_col = col_create(colp, left, left + LOGIN_WIDTH, "User\n------");
    left += LOGIN_WIDTH + 2;

    if (!option_terse_get())
    {
	name_col = col_create(colp, left, 0, "Full Name\n-----------");
    }

    //
    // list the project's developers
    //
    for (j = 0; ; ++j)
    {
	string_ty *logname = project_developer_nth(pp, j);
	if (!logname)
	    break;
	login_col->fputs(logname);
	if (name_col)
	    name_col->fputs(user_full_name(logname));
	col_eoln(colp);
    }

    //
    // clean up and go home
    //
    col_close(colp);
    project_free(pp);
    trace(("}\n"));
}
