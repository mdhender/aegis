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
// MANIFEST: functions to manipulate historys
//

#include <ael/change/inappropriat.h>
#include <ael/column_width.h>
#include <ael/project/history.h>
#include <change.h>
#include <col.h>
#include <cstate.h>
#include <error.h> // for assert
#include <option.h>
#include <output.h>
#include <project.h>
#include <project/history.h>
#include <str_list.h>
#include <trace.h>
#include <user.h>


void
list_project_history(string_ty *project_name, long change_number,
    string_list_ty *args)
{
    output_ty       *name_col = 0;
    output_ty       *delta_col = 0;
    output_ty       *date_col = 0;
    output_ty       *change_col = 0;
    output_ty       *description_col = 0;
    size_t          j, k;
    project_ty      *pp;
    string_ty       *line1;
    int             left;
    col_ty          *colp;

    trace(("list_project_history()\n{\n"));
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
    col_title(colp, line1->str_text, "History");
    str_free(line1);

    // the delta name column is the whole page wide
    name_col = col_create(colp, 0, 0, (const char *)0);

    left = 0;
    delta_col = col_create(colp, left, left + CHANGE_WIDTH, "Delta\n-------");
    left += CHANGE_WIDTH + 1;

    if (!option_terse_get())
    {
	date_col =
	    col_create
	    (
	       	colp,
	       	left,
	       	left + WHEN_WIDTH,
	       	"Date and Time\n---------------"
	    );
	left += WHEN_WIDTH + 1;

	change_col =
	    col_create(colp, left, left + CHANGE_WIDTH, "Change\n-------");
	left += CHANGE_WIDTH + 1;

	description_col =
	    col_create(colp, left, 0, "Description\n-------------");
    }

    //
    // list the project's successful integrations
    //
    for (j = 0; ; ++j)
    {
	long		cn;
	long		dn;
	string_list_ty	name;

	if (!project_history_nth(pp, j, &cn, &dn, &name))
	    break;
	if (!option_terse_get() && name.nstrings)
	{
	    col_need(colp, 4);
	    name_col->fprintf("Name%s: ", (name.nstrings == 1 ? "" : "s"));
	    for (k = 0; k < name.nstrings; ++k)
	    {
		if (k)
		    name_col->fputs(", ");
		name_col->fprintf("\"%s\"", name.string[k]->str_text);
	    }

	    //
	    // If we don't eoln here, and there are lots
	    // of names, then they get intermingled with
	    // the date and description lines, and it
	    // looks weird.
	    //
	    col_eoln(colp);
	}
	delta_col->fprintf("%4ld", dn);
	if (!option_terse_get())
	{
	    cstate_ty       *cstate_data;
	    time_t          t;
	    change_ty       *cp;

	    cp = change_alloc(pp, cn);
	    change_bind_existing(cp);
	    cstate_data = change_cstate_get(cp);
	    t =
		cstate_data->history->list
		[
			cstate_data->history->length - 1
		]->when;
	    date_col->fputs(ctime(&t));
	    change_col->fprintf("%4ld", cn);
	    assert(cstate_data->brief_description);
	    description_col->fputs(cstate_data->brief_description);
	    change_free(cp);
	}
	col_eoln(colp);
    }

    //
    // clean up and go home
    //
    col_close(colp);
    project_free(pp);
    trace(("}\n"));
}
