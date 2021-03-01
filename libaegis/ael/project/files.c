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
 * MANIFEST: functions to manipulate filess
 */

#include <ael/column_width.h>
#include <ael/formeditnum.h>
#include <ael/project/files.h>
#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <col.h>
#include <error.h> /* for assert */
#include <option.h>
#include <output.h>
#include <project.h>
#include <project/file.h>
#include <trace.h>
#include <user.h>


void
list_project_files(string_ty *project_name, long change_number)
{
    project_ty	    *pp;
    change_ty	    *cp;
    user_ty	    *up;
    output_ty	    *usage_col =    0;
    output_ty	    *edit_col =	    0;
    output_ty	    *file_name_col = 0;
    int		    j;
    string_ty	    *line1;
    int		    left;
    col_ty	    *colp;

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
    colp = col_open((string_ty *)0);
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
    col_title(colp, line1->str_text, "List of Project's Files");
    str_free(line1);

    left = 0;
    if (!option_terse_get())
    {
	usage_col = col_create(colp, left, left + USAGE_WIDTH, "Type\n-------");
	left += USAGE_WIDTH + 1;

	edit_col = col_create(colp, left, left + EDIT_WIDTH, "Edit\n-------");
	left += EDIT_WIDTH + 1;
    }
    file_name_col = col_create(colp, left, 0, "File Name\n-----------");

    /*
     * list the project's files
     */
    for (j = 0;; ++j)
    {
	fstate_src	src_data;

	src_data = project_file_nth(pp, j, view_path_simple);
	if (!src_data)
	    break;
	if (src_data->deleted_by && !option_verbose_get())
	    continue;
	if (cp && change_file_find(cp, src_data->file_name))
	    continue;
	if (option_terse_get())
	{
	    if (src_data->deleted_by)
		continue;
	}
	else
	{
	    output_fputs(usage_col, file_usage_ename(src_data->usage));
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
			src_data->file_name,
			view_path_extreme
		    );
		if (psrc_data && psrc_data->edit)
		{
		    assert(psrc_data->edit->revision);
		    output_fprintf
		    (
			edit_col,
			" (%s)",
			psrc_data->edit->revision->str_text
		    );
		}
	    }
	}
	assert(src_data->file_name);
	output_put_str(file_name_col, src_data->file_name);
	if (src_data->about_to_be_created_by)
	{
	    output_end_of_line(file_name_col);
	    output_fprintf
	    (
		file_name_col,
		"About to be created by change %ld.",
		magic_zero_decode(src_data->about_to_be_created_by)
	    );
	}
	if (src_data->deleted_by)
	{
	    output_end_of_line(file_name_col);
	    output_fprintf
	    (
		file_name_col,
		"Deleted by change %ld.",
		magic_zero_decode(src_data->deleted_by)
	    );
	}
	if (src_data->locked_by)
	{
	    output_end_of_line(file_name_col);
	    output_fprintf
	    (
		file_name_col,
		"Locked by change %ld.",
		magic_zero_decode(src_data->locked_by)
	    );
	}
	if (src_data->about_to_be_copied_by)
	{
	    output_end_of_line(file_name_col);
	    output_fprintf
	    (
		file_name_col,
		"About to be copied by change %ld.",
		magic_zero_decode(src_data->about_to_be_copied_by)
	    );
	}
	if (src_data->move)
	{
	    output_end_of_line(file_name_col);
	    output_fputs(file_name_col, "Moved ");
	    if (src_data->action == file_action_create)
		output_fputs(file_name_col, "from ");
	    else
		output_fputs(file_name_col, "to ");
	    output_fputs(file_name_col, src_data->move->str_text);
	}
	col_eoln(colp);
    }

    /*
     * clean up and go home
     */
    col_close(colp);
    project_free(pp);
    if (cp)
	change_free(cp);
    user_free(up);
    trace(("}\n"));
}
