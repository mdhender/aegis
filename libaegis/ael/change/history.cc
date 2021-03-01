//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001, 2003-2006 Peter Miller;
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

#include <libaegis/ael/change/history.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/aer/func/now.h>
#include <libaegis/change.h>
#include <libaegis/col.h>
#include <common/error.h> // for assert
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/user.h>


void
list_change_history(string_ty *project_name, long change_number,
    string_list_ty *args)
{
    cstate_ty       *cstate_data;
    project_ty      *pp;
    change_ty       *cp;
    user_ty         *up;
    output_ty       *what_col;
    output_ty       *when_col;
    output_ty       *who_col;
    output_ty       *why_col;
    size_t          j;
    string_ty       *line1;
    int             left;
    col_ty          *colp;

    //
    // locate project data
    //
    trace(("list_change_history()\n{\n"));
    if (!project_name)
	    project_name = user_default_project();
    else
	    project_name = str_copy(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_executing(pp);

    //
    // locate change data
    //
    if (!change_number)
	    change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    cstate_data = change_cstate_get(cp);

    //
    // create the columns
    //
    colp = col_open((string_ty *)0);
    line1 =
	    str_format
	    (
		    "Project \"%s\"  Change %ld",
		    project_name_get(pp)->str_text,
		    magic_zero_decode(change_number)
	    );
    col_title(colp, line1->str_text, "History");
    str_free(line1);

    left = 0;
    what_col = col_create(colp, left, left + WHAT_WIDTH, "What\n------");
    left += WHAT_WIDTH + 1;

    when_col = col_create(colp, left, left + WHEN_WIDTH, "When\n------");
    left += WHEN_WIDTH + 1;

    who_col = col_create(colp, left, left + WHO_WIDTH, "Who\n-----");
    left += WHO_WIDTH + 1;

    why_col = col_create(colp, left, 0, "Comment\n---------");

    //
    // list the history
    //
    for (j = 0; j < cstate_data->history->length; ++j)
    {
	cstate_history_ty *history_data;
	time_t          t;

	history_data = cstate_data->history->list[j];
	what_col->fputs(cstate_history_what_ename(history_data->what));
	t = history_data->when;
	when_col->fputs(ctime(&t));
	who_col->fputs(history_data->who->str_text);
	if (history_data->why)
	    why_col->fputs(history_data->why->str_text);
	if (history_data->what != cstate_history_what_integrate_pass)
	{
	    time_t          finish;

	    if (j + 1 < cstate_data->history->length)
		finish = cstate_data->history->list[j + 1]->when;
	    else
		time(&finish);
	    if (finish - t >= ELAPSED_TIME_THRESHOLD)
	    {
		why_col->end_of_line();
		why_col->fprintf
		(
	    	    "Elapsed time: %5.3f days.\n",
	    	    working_days(t, finish)
		);
	    }
	}
	col_eoln(colp);
    }

    //
    // clean up and go home
    //
    col_close(colp);
    change_free(cp);
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}
