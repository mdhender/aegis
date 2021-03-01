//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2004 Peter Miller;
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
// MANIFEST: functions to manipulate filess
//

#include <ael/attribu_list.h>
#include <ael/build_header.h>
#include <ael/change/files.h>
#include <ael/column_width.h>
#include <ael/formeditnum.h>
#include <change.h>
#include <change/file.h>
#include <col.h>
#include <error.h> // for assert
#include <option.h>
#include <output.h>
#include <project.h>
#include <project/file.h>
#include <str_list.h>
#include <symtab.h>
#include <trace.h>
#include <user.h>


static void
output_reaper(void *p)
{
    output_ty       *op;

    op = (output_ty *)p;
    output_delete(op);
}


void
list_change_files(string_ty *project_name, long change_number,
    string_list_ty *args)
{
    project_ty	    *pp;
    change_ty	    *cp;
    user_ty	    *up;
    output_ty	    *usage_col =    0;
    output_ty	    *action_col =   0;
    output_ty	    *edit_col =	    0;
    output_ty	    *file_name_col = 0;
    int		    j;
    string_ty	    *line1;
    int		    left;
    col_ty	    *colp;
    symtab_ty       *attr_col_stp;

    //
    // locate project data
    //
    trace(("list_change_files()\n{\n"));
    attr_col_stp = 0;
    if (!project_name)
	project_name = user_default_project();
    else
	project_name = str_copy(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

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
    col_title(colp, line1->str_text, "List of Change's Files");
    str_free(line1);

    left = 0;
    if (!option_terse_get())
    {
	usage_col = col_create(colp, left, left + USAGE_WIDTH, "Type\n-------");
	left += USAGE_WIDTH + 1;

	action_col =
	    col_create(colp, left, left + ACTION_WIDTH, "Action\n--------");
	left += ACTION_WIDTH + 1;

	edit_col = col_create(colp, left, left + EDIT_WIDTH, "Edit\n-------");
	left += EDIT_WIDTH + 1;

	attr_col_stp = symtab_alloc(5);
	attr_col_stp->reap = output_reaper;
	for (j = 0;; ++j)
	{
	    fstate_src_ty   *src_data;

	    src_data = change_file_nth(cp, j, view_path_first);
	    if (!src_data)
		break;
	    assert(src_data->file_name);
	    if (src_data->attribute)
	    {
		size_t          k;

		for (k = 0; k < src_data->attribute->length; ++k)
		{
		    attributes_ty   *ap;

		    ap = src_data->attribute->list[k];
		    if (ael_attribute_listable(ap))
		    {
			void            *p;

			p = symtab_query(attr_col_stp, ap->name);
			if (!p)
			{
			    string_ty       *s;
			    output_ty       *op;

			    s = ael_build_header(ap->name);
			    op =
				col_create
				(
				    colp,
				    left,
				    left + ATTR_WIDTH,
				    s->str_text
				);
			    str_free(s);
			    symtab_assign(attr_col_stp, ap->name, op);
			    left += ATTR_WIDTH + 1;
			}
		    }
		}
	    }
	}
    }
    file_name_col = col_create(colp, left, 0, "File Name\n-----------");

    //
    // list the change's files
    //
    for (j = 0;; ++j)
    {
	fstate_src_ty   *src_data;
	fstate_src_ty	*psrc_data;

	src_data = change_file_nth(cp, j, view_path_first);
	if (!src_data)
	    break;
	assert(src_data->file_name);
	psrc_data = 0;
	if (option_terse_get())
	{
	    switch (src_data->action)
	    {
	    case file_action_remove:
		continue;

	    case file_action_create:
	    case file_action_modify:
	    case file_action_insulate:
	    case file_action_transparent:
		break;
	    }
	}
	else
	{
	    output_fputs(usage_col, file_usage_ename(src_data->usage));
	    output_fputs(action_col, file_action_ename(src_data->action));
	    list_format_edit_number(edit_col, src_data);
	    psrc_data =
		project_file_find(pp, src_data->file_name, view_path_extreme);
	    if
	    (
		change_is_being_developed(cp)
	    &&
		!change_file_up_to_date(pp, src_data)
	    )
	    {
		//
		// The current head revision of the branch may not equal
		// the version ``originally'' copied.
		//
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
	    if (src_data->edit_origin_new)
	    {
		//
		// The ``cross branch merge'' version.
		//
		assert(src_data->edit_origin_new->revision);
		output_end_of_line(edit_col);
		output_fprintf
		(
		    edit_col,
		    "{cross %4s}",
		    src_data->edit_origin_new->revision->str_text
		);
	    }
	}
	output_put_str(file_name_col, src_data->file_name);
	if
	(
	    change_is_being_developed(cp)
	&&
	    psrc_data
	&&
	    psrc_data->locked_by
	&&
	    psrc_data->locked_by != change_number
	)
	{
	    output_end_of_line(file_name_col);
	    output_fprintf
	    (
		file_name_col,
		"Locked by change %ld.",
		magic_zero_decode(psrc_data->locked_by)
	    );
	}
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
	    switch (src_data->action)
	    {
	    case file_action_create:
		output_end_of_line(file_name_col);
		output_fputs(file_name_col, "Moved from ");
		output_fputs(file_name_col, src_data->move->str_text);
		break;

	    case file_action_remove:
		output_end_of_line(file_name_col);
		output_fputs(file_name_col, "Moved to ");
		output_fputs(file_name_col, src_data->move->str_text);
		break;

	    case file_action_modify:
	    case file_action_insulate:
	    case file_action_transparent:
		assert(0);
		break;
	    }
	}
	if (attr_col_stp && src_data->attribute)
	{
	    size_t          k;

	    for (k = 0; k < src_data->attribute->length; ++k)
	    {
		attributes_ty   *ap;

		ap = src_data->attribute->list[k];
		if (ap->name && ap->value)
		{
		    output_ty       *op;

		    op = (output_ty *)symtab_query(attr_col_stp, ap->name);
		    if (op)
			output_put_str(op, ap->value);
		}
	    }
	}
	col_eoln(colp);
    }

    //
    // clean up and go home
    //
    if (attr_col_stp)
	symtab_free(attr_col_stp);
    col_close(colp);
    project_free(pp);
    change_free(cp);
    user_free(up);
    trace(("}\n"));
}