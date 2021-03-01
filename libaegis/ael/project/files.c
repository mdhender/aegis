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

#include <ael/column_width.h>
#include <ael/formeditnum.h>
#include <ael/project/files.h>
#include <change.h>
#include <change_bran.h>
#include <change/file.h>
#include <col.h>
#include <error.h> /* for assert */
#include <option.h>
#include <project.h>
#include <project/file.h>
#include <trace.h>
#include <user.h>


void
list_project_files(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	project_ty	*pp;
	change_ty	*cp;
	user_ty		*up;
	int		usage_col = 0;
	int		edit_col = 0;
	int		file_name_col = 0;
	int		j;
	string_ty	*line1;
	int		left;

	/*
	 * locate project data
	 */
	trace(("list_project_files()\n{\n"));
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
	if (change_number)
	{
		cp = change_alloc(pp, change_number);
		change_bind_existing(cp);
	}
	else
		cp = 0;

	/*
	 * create the columns
	 */
	col_open((char *)0);
	if (change_number)
	{
		line1 =
			str_format
			(
				"Project \"%S\"  Change %ld",
				project_name_get(pp),
				magic_zero_decode(change_number)
			);
	}
	else
		line1 = str_format("Project \"%S\"", project_name_get(pp));
	col_title(line1->str_text, "List of Project's Files");
	str_free(line1);

	left = 0;
	if (!option_terse_get())
	{
		usage_col = col_create(left, left + USAGE_WIDTH);
		left += USAGE_WIDTH + 1;
		col_heading(usage_col, "Type\n-------");

		edit_col = col_create(left, left + EDIT_WIDTH);
		left += EDIT_WIDTH + 1;
		col_heading(edit_col, "Edit\n-------");
	}
	file_name_col = col_create(left, 0);
	col_heading(file_name_col, "File Name\n-----------");

	/*
	 * list the project's files
	 */
	for (j = 0; ; ++j)
	{
		fstate_src	src_data;

		src_data = project_file_nth(pp, j);
		if (!src_data)
			break;
		if
		(
			(
				src_data->about_to_be_created_by
			||
				src_data->deleted_by
			)
		&&
			!option_verbose_get()
		)
			continue;
		if (cp && change_file_find(cp, src_data->file_name))
			continue;
		if (option_terse_get())
		{
			if (src_data->deleted_by)
				continue;
			if (src_data->about_to_be_created_by)
				continue;
		}
		else
		{
			col_puts
			(
				usage_col,
				file_usage_ename(src_data->usage)
			);
			list_format_edit_number(edit_col, src_data);
			if
			(
				pp->parent
			&&
				change_is_a_branch(project_change_get(pp))
			&&
				!change_file_up_to_date(pp->parent, src_data)
			)
			{
				fstate_src	psrc_data;

				psrc_data =
					project_file_find
					(
						pp->parent,
						src_data->file_name
					);
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
		}
		assert(src_data->file_name);
		col_puts
		(
			file_name_col,
			src_data->file_name->str_text
		);
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
	if (cp)
		change_free(cp);
	user_free(up);
	trace(("}\n"));
}
