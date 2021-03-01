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
 * MANIFEST: functions to manipulate historys
 */

#include <ael/change/history.h>
#include <ael/column_width.h>
#include <aer/func/now.h>
#include <change.h>
#include <col.h>
#include <error.h> /* for assert */
#include <project.h>
#include <trace.h>
#include <user.h>


void
list_change_history(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	cstate		cstate_data;
	project_ty	*pp;
	change_ty	*cp;
	user_ty		*up;
	int		what_col;
	int		when_col;
	int		who_col;
	int		why_col;
	int		j;
	string_ty	*line1;
	int		left;

	/*
	 * locate project data
	 */
	trace(("list_change_history()\n{\n"));
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
	assert(change_file_nth(cp, (size_t)0));

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 =
		str_format
		(
			"Project \"%S\"  Change %ld",
			project_name_get(pp),
			magic_zero_decode(change_number)
		);
	col_title(line1->str_text, "History");
	str_free(line1);

	left = 0;
	what_col = col_create(left, left + WHAT_WIDTH);
	left += WHAT_WIDTH + 1;
	col_heading(what_col, "What\n------");

	when_col = col_create(left, left + WHEN_WIDTH);
	left += WHEN_WIDTH + 1;
	col_heading(when_col, "When\n------");

	who_col = col_create(left, left + WHO_WIDTH);
	left += WHO_WIDTH + 1;
	col_heading(who_col, "Who\n-----");

	why_col = col_create(left, 0);
	col_heading(why_col, "Comment\n---------");

	/*
	 * list the history
	 */
	for (j = 0; j < cstate_data->history->length; ++j)
	{
		cstate_history	history_data;
		time_t		t;

		history_data = cstate_data->history->list[j];
		col_puts
		(
			what_col,
			cstate_history_what_ename(history_data->what)
		);
		t = history_data->when;
		col_puts(when_col, ctime(&t));
		col_puts(who_col, history_data->who->str_text);
		if (history_data->why)
			col_puts(why_col, history_data->why->str_text);
		if (history_data->what != cstate_history_what_integrate_pass)
		{
			time_t	finish;

			if (j + 1 < cstate_data->history->length)
				finish =
					cstate_data->history->list[j + 1]->when;
			else
				time(&finish);
			if (finish - t >= ELAPSED_TIME_THRESHOLD)
			{
				col_bol(why_col);
				col_printf
				(
					why_col,
					"Elapsed time: %5.3f days.\n",
					working_days(t, finish)
				);
			}
		}
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace(("}\n"));
}
