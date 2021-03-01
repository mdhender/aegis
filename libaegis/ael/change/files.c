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
 * MANIFEST: functions to manipulate filess
 */

#include <ael/change/files.h>
#include <ael/column_width.h>
#include <ael/formeditnum.h>
#include <change.h>
#include <change_file.h>
#include <col.h>
#include <error.h> /* for assert */
#include <option.h>
#include <project.h>
#include <project_file.h>
#include <trace.h>
#include <user.h>


void
list_change_files(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	cstate		cstate_data;
	project_ty	*pp;
	change_ty	*cp;
	user_ty		*up;
	int		usage_col = 0;
	int		action_col = 0;
	int		edit_col = 0;
	int		file_name_col = 0;
	int		j;
	string_ty	*line1;
	int		left;

	/*
	 * locate project data
	 */
	trace(("list_change_files()\n{\n"/*}*/));
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
	col_title(line1->str_text, "List of Change's Files");
	str_free(line1);

	left = 0;
	if (!option_terse_get())
	{
		usage_col = col_create(left, left + USAGE_WIDTH);
		left += USAGE_WIDTH + 1;
		col_heading(usage_col, "Type\n-------");

		action_col = col_create(left, left + ACTION_WIDTH);
		left += ACTION_WIDTH + 1;
		col_heading(action_col, "Action\n--------");

		edit_col = col_create(left, left + EDIT_WIDTH);
		left += EDIT_WIDTH + 1;
		col_heading(edit_col, "Edit\n-------");
	}
	file_name_col = col_create(left, 0);
	col_heading(file_name_col, "File Name\n-----------");

	/*
	 * list the change's files
	 */
	for (j = 0; ; ++j)
	{
		fstate_src	src_data;
		fstate_src	psrc_data;

		src_data = change_file_nth(cp, j);
		if (!src_data)
			break;
		assert(src_data->file_name);
		psrc_data = 0;
		if (option_terse_get())
		{
			if (src_data->action == file_action_remove)
				continue;
		}
		else
		{
			col_puts
			(
				usage_col,
				file_usage_ename(src_data->usage)
			);
			col_puts
			(
				action_col,
				file_action_ename(src_data->action)
			);
			list_format_edit_number(edit_col, src_data);
			psrc_data = project_file_find(pp, src_data->file_name);
			if
			(
			      cstate_data->state == cstate_state_being_developed
			&&
				!change_file_up_to_date(pp, src_data)
			)
			{
				/*
				 * The current head revision of the
				 * branch may not equal the version
				 * ``originally'' copied.
				 */
				if (psrc_data && psrc_data->edit_number)
				{
					col_printf
					(
						edit_col,
						" (%s)",
						psrc_data->edit_number->str_text
					);
				}
			}
			if (src_data->edit_number_origin_new)
			{
				/*
				 * The ``cross branch merge'' version.
				 */
				col_bol(edit_col);
				col_printf
				(
					edit_col,
					"{cross %4s}",
				      src_data->edit_number_origin_new->str_text
				);
			}
		}
		col_puts
		(
			file_name_col,
			src_data->file_name->str_text
		);
		if
		(
			cstate_data->state == cstate_state_being_developed
		&&
			psrc_data
		&&
			psrc_data->locked_by
		&&
			psrc_data->locked_by != change_number
		)
		{
			col_printf
			(
				file_name_col,
				"\nLocked by change %ld.",
				magic_zero_decode(psrc_data->locked_by)
			);
		}
		if (src_data->about_to_be_created_by)
		{
			col_bol(file_name_col);
			col_printf
			(
				file_name_col,
				"About to be created by change %ld.",
				magic_zero_decode(src_data->about_to_be_created_by)
			);
		}
		if (src_data->deleted_by)
		{
			col_bol(file_name_col);
			col_printf
			(
				file_name_col,
				"Deleted by change %ld.",
				magic_zero_decode(src_data->deleted_by)
			);
		}
		if (src_data->locked_by)
		{
			col_bol(file_name_col);
			col_printf
			(
				file_name_col,
				"Locked by change %ld.",
				magic_zero_decode(src_data->locked_by)
			);
		}
		if (src_data->about_to_be_copied_by)
		{
			col_bol(file_name_col);
			col_printf
			(
				file_name_col,
				"About to be copied by change %ld.",
				magic_zero_decode(src_data->about_to_be_copied_by)
			);
		}
		if (src_data->move)
		{
			col_bol(file_name_col);
			col_puts(file_name_col, "Moved ");
			if (src_data->action == file_action_create)
				col_puts(file_name_col, "from ");
			else
				col_puts(file_name_col, "to ");
			col_puts(file_name_col, src_data->move->str_text);
		}
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}
