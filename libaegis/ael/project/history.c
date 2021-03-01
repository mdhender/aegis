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

#include <ael/change/inappropriat.h>
#include <ael/column_width.h>
#include <ael/project/history.h>
#include <change.h>
#include <col.h>
#include <cstate.h>
#include <error.h> /* for assert */
#include <option.h>
#include <project.h>
#include <project_hist.h>
#include <str_list.h>
#include <trace.h>
#include <user.h>


void
list_project_history(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	int		name_col = 0;
	int		delta_col = 0;
	int		date_col = 0;
	int		change_col = 0;
	int		description_col = 0;
	size_t		j, k;
	project_ty	*pp;
	string_ty	*line1;
	int		left;

	trace(("list_project_history()\n{\n"/*}*/));
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
	col_title(line1->str_text, "History");
	str_free(line1);

	/* the delta name column is the whole page wide */
	name_col = col_create(0, 0);

	left = 0;
	delta_col = col_create(left, left + CHANGE_WIDTH);
	left += CHANGE_WIDTH + 1;
	col_heading(delta_col, "Delta\n-------");

	if (!option_terse_get())
	{
		date_col = col_create(left, left + WHEN_WIDTH);
		left += WHEN_WIDTH + 1;
		col_heading(date_col, "Date and Time\n---------------");

		change_col = col_create(left, left + CHANGE_WIDTH);
		left += CHANGE_WIDTH + 1;
		col_heading(change_col, "Change\n-------");

		description_col = col_create(left, 0);
		col_heading(description_col, "Description\n-------------");
	}

	/*
	 * list the project's successful integrations
	 */
	for (j = 0; ; ++j)
	{
		long		cn;
		long		dn;
		string_list_ty	name;

		if (!project_history_nth(pp, j, &cn, &dn, &name))
			break;
		if
		(
			!option_terse_get()
		&&
			name.nstrings
		)
		{
			col_need(4);
			col_printf
			(
				name_col,
				"Name%s: ",
				(name.nstrings == 1 ? "" : "s")
			);
			for (k = 0; k < name.nstrings; ++k)
			{
				if (k)
					col_printf(name_col, ", ");
				col_printf
				(
					name_col,
					"\"%s\"",
					name.string[k]->str_text
				);
			}
		}
		col_printf(delta_col, "%4ld", dn);
		if (!option_terse_get())
		{
			cstate		cstate_data;
			time_t		t;
			change_ty	*cp;

			cp = change_alloc(pp, cn);
			change_bind_existing(cp);
			cstate_data = change_cstate_get(cp);
			t =
				cstate_data->history->list
				[
					cstate_data->history->length - 1
				]->when;
			col_puts(date_col, ctime(&t));
			col_printf
			(
				change_col,
				"%4ld",
				cn
			);
			assert(cstate_data->brief_description);
			col_puts
			(
				description_col,
				cstate_data->brief_description->str_text
			);
			change_free(cp);
		}
		col_eoln();
		string_list_destructor(&name);
	}

	/*
	 * clean up and go home
	 */
	col_close();
	project_free(pp);
	trace((/*{*/"}\n"));
}
