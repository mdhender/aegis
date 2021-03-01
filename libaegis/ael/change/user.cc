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
// MANIFEST: functions to manipulate users
//

#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/change/user.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/project/inappropriat.h>
#include <libaegis/change.h>
#include <libaegis/col.h>
#include <common/error.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <common/str_list.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/user.h>


void
list_user_changes(struct string_ty *project_name, long change_number,
    string_list_ty *args)
{
    user_ty	    *up;
    output_ty	    *project_col;
    output_ty	    *change_col;
    output_ty	    *state_col;
    output_ty	    *description_col;
    string_ty	    *s;
    string_list_ty  name;
    size_t	    j;
    int		    left;
    col_ty	    *colp;
    string_ty       *login = 0;

    trace(("list_user_changes()\n{\n"));
    if (project_name)
	list_project_inappropriate();
    if (change_number)
	list_change_inappropriate();

    //
    // get the list of projects
    //
    project_list_get(&name);
    if (!name.nstrings)
	goto done;

    if (!args->nstrings)
    {
        //
	// No user name is provided, use the current user.
	//
        up = user_executing((project_ty *)0);
    }
    else
    {
        //
	// Use the user name supplied by the caller.
	//
        login = args->string[0];
        up = user_symbolic((project_ty *)0, login);
    }

    //
    // open listing
    //
    colp = col_open((string_ty *)0);
    s =
	str_format
	(
	    "Owned by %s <%s>",
	    up->full_name->str_text,
	    user_name(up)->str_text
	);
    user_free(up);
    col_title(colp, "List of Changes", s->str_text);
    str_free(s);

    //
    // create the columns
    //
    left = 0;
    project_col =
	col_create(colp, left, left + PROJECT_WIDTH, "Project\n----------");
    left += PROJECT_WIDTH + 1;
    change_col = col_create(colp, left, left + CHANGE_WIDTH, "Change\n------");
    left += CHANGE_WIDTH + 1;
    state_col = col_create(colp, left, left + STATE_WIDTH, "State\n----------");
    left += STATE_WIDTH + 1;
    description_col = col_create(colp, left, 0, "Description\n-------------");

    //
    // for each project, see if the current user
    // is working on any of them.
    //
    for (j = 0; j < name.nstrings; ++j)
    {
	project_ty	*pp;
	int		err;
	long		n;

	//
	// locate the project,
	// and make sure we are allowed to look at it
	//
	pp = project_alloc(name.string[j]);
	pp->bind_existing();
	err = project_is_readable(pp);
	if (err)
	{
	    project_free(pp);
	    continue;
	}

	//
	// bind a user to that project
	//
        if (!login)
            up = user_executing(pp);
        else
            up = user_symbolic(pp, login);

	//
	// for each change within this project the user
	// is working on emit a line of information
	//
	for (n = 0;; ++n)
	{
	    change_ty	    *cp;
	    cstate_ty	    *cstate_data;

	    if (!user_own_nth(up, project_name_get(pp), n, &change_number))
		break;

	    //
	    // locate change data
	    //
	    cp = change_alloc(pp, change_number);
	    change_bind_existing(cp);

	    //
	    // emit the info
	    //
	    project_col->fputs(project_name_get(pp)->str_text);
	    change_col->fprintf("%4ld", magic_zero_decode(change_number));
	    cstate_data = change_cstate_get(cp);
	    state_col->fputs(cstate_state_ename(cstate_data->state));
	    if (cstate_data->brief_description)
	    {
		description_col->fputs
		(
		    cstate_data->brief_description->str_text
		);
	    }
	    col_eoln(colp);

	    //
	    // release change and project
	    //
	    change_free(cp);
	}

	//
	// free user and project
	//
        str_free(login);
	user_free(up);
	project_free(pp);
    }

    //
    // clean up and go home
    //
    col_close(colp);
    done:
    trace(("}\n"));
}
