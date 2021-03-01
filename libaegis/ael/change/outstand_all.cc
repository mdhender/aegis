//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2006 Peter Miller;
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
// MANIFEST: functions to manipulate outstand_alls
//

#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/change/outstand_all.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/project/inappropriat.h>
#include <libaegis/change.h>
#include <libaegis/col.h>
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <common/str_list.h>
#include <common/trace.h>


void
list_outstanding_changes_all(string_ty *project_name, long change_number,
    string_list_ty *args)
{
    output_ty       *project_col = 0;
    output_ty       *number_col = 0;
    output_ty       *state_col = 0;
    output_ty       *description_col = 0;
    size_t          j, k;
    string_list_ty  name;
    int             left;
    col_ty          *colp;

    trace(("list_outstanding_changes_all()\n{\n"));
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
    col_title(colp, "List of Outstanding Changes", "for all projects");

    left = 0;
    project_col =
	col_create(colp, left, left + PROJECT_WIDTH, "Project\n---------");
    left += PROJECT_WIDTH + 1;

    number_col = col_create(colp, left, left + CHANGE_WIDTH, "Change\n------");
    left += CHANGE_WIDTH + 1;

    if (!option_terse_get())
    {
	state_col =
	    col_create(colp, left, left + STATE_WIDTH, "State\n-------");
	left += STATE_WIDTH + 1;

	description_col =
	    col_create(colp, left, 0, "Description\n-------------");
    }

    //
    // scan each project
    //
    for (j = 0; j < name.nstrings; ++j)
    {
	project_ty	*pp;
	int		err;

	pp = project_alloc(name.string[j]);
	pp->bind_existing();

	//
	// make sure we have permission
	//
	err = project_is_readable(pp);
	if (err)
	{
	    project_free(pp);
	    continue;
	}

	//
	// list the project's changes
	//
	for (k = 0; ; ++k)
	{
	    cstate_ty       *cstate_data;
	    change_ty       *cp;

	    //
	    // make sure the change is not completed
	    //
	    if (!project_change_nth(pp, k, &change_number))
		break;
	    cp = change_alloc(pp, change_number);
	    change_bind_existing(cp);
	    cstate_data = change_cstate_get(cp);
	    if (cstate_data->state == cstate_state_completed)
	    {
		change_free(cp);
		continue;
	    }

	    //
	    // print the details
	    //
	    project_col->fputs(project_name_get(pp));
	    number_col->fprintf("%4ld", magic_zero_decode(change_number));
	    if (state_col)
	    {
		state_col->fputs(cstate_state_ename(cstate_data->state));
		if
		(
		    option_verbose_get()
		&&
		    cstate_data->state == cstate_state_being_developed
		)
		{
		    state_col->end_of_line();
		    state_col->fputs(change_developer_name(cp));
		}
		if
		(
		    option_verbose_get()
		&&
		    cstate_data->state == cstate_state_being_integrated
		)
		{
		    state_col->end_of_line();
		    state_col->fputs(change_integrator_name(cp));
		}
	    }
	    if (description_col && cstate_data->brief_description)
	    {
		description_col->fputs(cstate_data->brief_description);
	    }
	    col_eoln(colp);

	    //
	    // At some point, will need to recurse
	    // if it is a branch and not a leaf.
	    //
	    change_free(cp);
	}
	project_free(pp);
    }

    //
    // clean up and go home
    //
    col_close(colp);
    trace(("}\n"));
}
