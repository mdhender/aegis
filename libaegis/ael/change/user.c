/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate users
 */

#include <ael/change/inappropriat.h>
#include <ael/change/user.h>
#include <ael/column_width.h>
#include <ael/project/inappropriat.h>
#include <change.h>
#include <col.h>
#include <output.h>
#include <project.h>
#include <str_list.h>
#include <trace.h>
#include <user.h>


void
list_user_changes(project_name, change_number)
    string_ty	    *project_name;
    long	    change_number;
{
    user_ty	    *up;
    output_ty	    *project_col;
    output_ty	    *change_col;
    output_ty	    *state_col;
    output_ty	    *description_col;
    string_ty	    *s;
    string_list_ty  name;
    long	    j;
    int		    left;
    col_ty	    *colp;

    trace(("list_user_changes()\n{\n"));
    if (project_name)
	list_project_inappropriate();
    if (change_number)
	list_change_inappropriate();

    /*
     * get the list of projects
     */
    project_list_get(&name);
    if (!name.nstrings)
	goto done;

    /*
     * open listing
     */
    colp = col_open((string_ty *)0);
    up = user_executing((project_ty *)0);
    s = str_format("Owned by %S <%S>", up->full_name, user_name(up));
    user_free(up);
    col_title(colp, "List of Changes", s->str_text);
    str_free(s);

    /*
     * create the columns
     */
    left = 0;
    project_col =
	col_create(colp, left, left + PROJECT_WIDTH, "Project\n----------");
    left += PROJECT_WIDTH + 1;
    change_col = col_create(colp, left, left + CHANGE_WIDTH, "Change\n------");
    left += CHANGE_WIDTH + 1;
    state_col = col_create(colp, left, left + STATE_WIDTH, "State\n----------");
    left += STATE_WIDTH + 1;
    description_col = col_create(colp, left, 0, "Description\n-------------");

    /*
     * for each project, see if the current user
     * is working on any of them.
     */
    for (j = 0; j < name.nstrings; ++j)
    {
	project_ty	*pp;
	int		err;
	long		n;

	/*
	 * locate the project,
	 * and make sure we are allowed to look at it
	 */
	pp = project_alloc(name.string[j]);
	project_bind_existing(pp);
	err = project_is_readable(pp);
	if (err)
	{
	    project_free(pp);
	    continue;
	}

	/*
	 * bind a user to that project
	 */
	up = user_executing(pp);

	/*
	 * for each change within this project the user
	 * is working on emit a line of information
	 */
	for (n = 0;; ++n)
	{
	    change_ty	    *cp;
	    cstate	    cstate_data;

	    if (!user_own_nth(up, project_name_get(pp), n, &change_number))
		break;

	    /*
	     * locate change data
	     */
	    cp = change_alloc(pp, change_number);
	    change_bind_existing(cp);

	    /*
	     * emit the info
	     */
	    output_fputs(project_col, project_name_get(pp)->str_text);
	    output_fprintf
	    (
		change_col,
		"%4ld",
		magic_zero_decode(change_number)
	    );
	    cstate_data = change_cstate_get(cp);
	    output_fputs(state_col, cstate_state_ename(cstate_data->state));
	    if (cstate_data->brief_description)
	    {
		output_fputs
		(
		    description_col,
		    cstate_data->brief_description->str_text
		);
	    }
	    col_eoln(colp);

	    /*
	     * release change and project
	     */
	    change_free(cp);
	}

	/*
	 * free user and project
	 */
	user_free(up);
	project_free(pp);
    }

    /*
     * clean up and go home
     */
    col_close(colp);
    done:
    trace(("}\n"));
}
