/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate file_historys
 */

#include <ael/change/file_history.h>
#include <ael/column_width.h>
#include <ael/formeditnum.h>
#include <change/branch.h>
#include <change/file.h>
#include <change.h>
#include <col.h>
#include <error.h> /* for assert */
#include <option.h>
#include <output.h>
#include <project/file.h>
#include <project/file/roll_forward.h>
#include <project.h>
#include <trace.h>
#include <user.h>


static string_ty *
change_brief_description_get(change_ty *cp)
{
    cstate	    cstate_data;

    cstate_data = change_cstate_get(cp);
    return cstate_data->brief_description;
}


static int
change_is_completed(change_ty *cp)
{
    cstate	    cstate_data;

    cstate_data = change_cstate_get(cp);
    return (cstate_data->state == cstate_state_completed);
}


#define VERSION_WIDTH 10


void
list_change_file_history(string_ty *project_name, long change_number)
{
    cstate	    cstate_data;
    project_ty	    *pp;
    change_ty	    *cp;
    user_ty	    *up;
    output_ty	    *usage_col;
    output_ty	    *action_col;
    output_ty	    *when_col;
    output_ty	    *file_name_col;
    int		    j;
    string_ty	    *line1;
    int		    left;
    col_ty	    *colp;
    output_ty	    *delta_col;
    output_ty	    *change_col;
    time_t	    when;
    output_ty	    *description_col;

    /*
     * locate project data
     */
    trace(("list_change_file_history()\n{\n"));
    if (!project_name)
	project_name = user_default_project();
    else
	project_name = str_copy(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    /*
     * locate user data
     */
    up = user_executing(pp);

    /*
     * locate change data
     */
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    cstate_data = change_cstate_get(cp);
    assert(change_file_nth(cp, 0));

    /*
     * Reconstruct the project file history.
     */
    when = change_completion_timestamp(cp);
    project_file_roll_forward(pp, when, option_verbose_get());

    /*
     * create the columns
     */
    colp = col_open((string_ty *)0);
    line1 =
	str_format
	(
	    "Project \"%S\"  Change %ld",
	    project_name_get(pp),
	    magic_zero_decode(change_number)
	);
    col_title(colp, line1->str_text, "Change File History");
    str_free(line1);

    file_name_col = col_create(colp, 0, 0, "File Name\n-----------");

    left = 2;
    usage_col = col_create(colp, left, left + USAGE_WIDTH, 0);
    left += USAGE_WIDTH + 1;
    action_col = col_create(colp, left, left + ACTION_WIDTH, 0);
    left += ACTION_WIDTH + 1;

    delta_col =
	col_create(colp, left, left + VERSION_WIDTH, "Delta\n---------");
    left += VERSION_WIDTH + 1;

    when_col =
	col_create
	(
	    colp,
	    left,
	    left + WHEN_WIDTH,
	    "Date and Time\n---------------"
	);
    left += WHEN_WIDTH + 1;

    change_col = col_create(colp, left, left + CHANGE_WIDTH, "Change\n-------");
    left += CHANGE_WIDTH + 1;

    description_col = col_create(colp, left, 0, "Description\n-------------");

    /*
     * list the change's files' histories
     */
    for (j = 0;; ++j)
    {
	fstate_src	src_data;
	file_event_list_ty *felp;
	size_t		k;
	int		usage_track;
	int		action_track;

	src_data = change_file_nth(cp, j);
	if (!src_data)
	    break;
	assert(src_data->file_name);
	col_need(colp, 4);

	output_fputs(file_name_col, src_data->file_name->str_text);
	col_eoln(colp);

	usage_track = -1;
	action_track = -1;

	felp = project_file_roll_forward_get(src_data->file_name);
	if (felp)
	{
	    for (k = 0; k < felp->length; ++k)
	    {
		fstate_src	src2_data;
		file_event_ty	*fep;
		string_ty	*s;

		fep = felp->item + k;
		s = change_version_get(fep->cp);
		output_fputs(delta_col, s->str_text);
		str_free(s);
		output_fprintf(change_col, "%4ld", fep->cp->number);
		src2_data = change_file_find(fep->cp, src_data->file_name);
		assert(src2_data);

		if (usage_track != src2_data->usage)
		{
		    output_fputs(usage_col, file_usage_ename(src2_data->usage));
		    usage_track = src2_data->usage;
		}
		if (action_track != src2_data->action)
		{
		    output_fputs
		    (
			action_col,
			file_action_ename(src2_data->action)
		    );
		    action_track = src2_data->action;
		}
		assert(src2_data->edit);
		assert(src2_data->edit->revision);
		output_fputs(when_col, ctime(&fep->when));
		output_fputs
		(
		    description_col,
		    change_brief_description_get(fep->cp)->str_text
		);
		col_eoln(colp);
	    }
	}

	/*
	 * Now output details of this change, as the "end"
	 * of the history.
	 */
	if (!change_is_completed(cp))
	{
	    output_fputs(delta_col, change_version_get(cp)->str_text);
	    output_fprintf(change_col, "%4ld", cp->number);
	    if (usage_track != src_data->usage)
	    {
		output_fputs(usage_col, file_usage_ename(src_data->usage));
	    }
	    if (action_track != src_data->action)
	    {
		output_fputs(action_col, file_action_ename(src_data->action));
	    }
	    output_fputs
	    (
		description_col,
		change_brief_description_get(cp)->str_text
	    );
	    col_eoln(colp);
	}
    }

    /*
     * clean up and go home
     */
    col_close(colp);
    project_free(pp);
    change_free(cp);
    user_free(up);
    trace(("}\n"));
}
