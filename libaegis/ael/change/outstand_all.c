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
 * MANIFEST: functions to manipulate outstand_alls
 */

#include <ael/change/inappropriat.h>
#include <ael/change/outstand_all.h>
#include <ael/column_width.h>
#include <ael/project/inappropriat.h>
#include <change.h>
#include <col.h>
#include <option.h>
#include <project.h>
#include <project_hist.h>
#include <str_list.h>
#include <trace.h>


void
list_outstanding_changes_all(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	int		project_col = 0;
	int		number_col = 0;
	int		state_col = 0;
	int		description_col = 0;
	int		j, k;
	string_list_ty	name;
	int		left;

	trace(("list_outstanding_changes_all()\n{\n"));
	if (project_name)
		list_project_inappropriate();
	if (change_number)
		list_change_inappropriate();

	/*
	 * list the projects
	 */
	project_list_get(&name);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	col_title("List of Outstanding Changes", "for all projects");

	left = 0;
	project_col = col_create(left, left + PROJECT_WIDTH);
	left += PROJECT_WIDTH + 1;
	col_heading(project_col, "Project\n---------");

	number_col = col_create(left, left + CHANGE_WIDTH);
	left += CHANGE_WIDTH + 1;
	col_heading(number_col, "Change\n------");

	if (!option_terse_get())
	{
		state_col = col_create(left, left + STATE_WIDTH);
		left += STATE_WIDTH + 1;
		col_heading(state_col, "State\n-------");

		description_col = col_create(left, 0);
		col_heading(description_col, "Description\n-------------");
	}

	/*
	 * scan each project
	 */
	for (j = 0; j < name.nstrings; ++j)
	{
		project_ty	*pp;
		int		err;

		pp = project_alloc(name.string[j]);
		project_bind_existing(pp);

		/*
		 * make sure we have permission
		 */
		err = project_is_readable(pp);
		if (err)
		{
			project_free(pp);
			continue;
		}

		/*
		 * list the project's changes
		 */
		for (k = 0; ; ++k)
		{
			cstate		cstate_data;
			change_ty	*cp;

			/*
			 * make sure the change is not completed
			 */
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

			/*
			 * print the details
			 */
			col_puts(project_col, project_name_get(pp)->str_text);
			col_printf
			(
				number_col,
				"%4ld",
				magic_zero_decode(change_number)
			);
			if (!option_terse_get())
			{
				col_puts
				(
					state_col,
					cstate_state_ename(cstate_data->state)
				);
				if
				(
					option_verbose_get()
				&&
			      cstate_data->state == cstate_state_being_developed
				)
				{
					col_bol(state_col);
					col_puts
					(
						state_col,
					     change_developer_name(cp)->str_text
					);
				}
				if
				(
					option_verbose_get()
				&&
			     cstate_data->state == cstate_state_being_integrated
				)
				{
					col_bol(state_col);
					col_puts
					(
						state_col,
					    change_integrator_name(cp)->str_text
					);
				}
				if (cstate_data->brief_description)
				{
					col_puts
					(
						description_col,
					cstate_data->brief_description->str_text
					);
				}
			}
			col_eoln();

			/*
			 * At some point, will need to recurse
			 * if it is a branch and not a leaf.
			 */
			change_free(cp);
		}
		project_free(pp);
	}

	/*
	 * clean up and go home
	 */
	col_close();
	trace(("}\n"));
}
