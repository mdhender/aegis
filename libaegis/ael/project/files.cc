//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2007 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//
// MANIFEST: functions to manipulate filess
//

#include <libaegis/ael/attribu_list.h>
#include <libaegis/ael/build_header.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/formeditnum.h>
#include <libaegis/ael/project/files.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/col.h>
#include <common/error.h> // for assert
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <common/str_list.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <libaegis/user.h>


void
list_project_files(string_ty *project_name, long change_number,
    string_list_ty *)
{
    project_ty	    *pp;
    change::pointer cp;
    user_ty::pointer up;
    output_ty	    *usage_col =    0;
    output_ty	    *edit_col =	    0;
    output_ty	    *file_name_col = 0;
    int		    j;
    string_ty	    *line1;
    int		    left;
    col	    *colp;
    symtab_ty       *attr_col_stp = 0;

    //
    // locate project data
    //
    trace(("list_project_files()\n{\n"));
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
    else
	project_name = str_copy(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // locate change data
    //
    if (change_number)
    {
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
    }
    else
	cp = 0;

    //
    // create the columns
    //
    colp = col::open((string_ty *)0);
    if (change_number)
    {
	line1 =
	    str_format
	    (
		"Project \"%s\"  Change %ld",
		project_name_get(pp)->str_text,
		magic_zero_decode(change_number)
	    );
    }
    else
	line1 = str_format("Project \"%s\"", project_name_get(pp)->str_text);
    colp->title(line1->str_text, "List of Project's Files");
    str_free(line1);

    left = 0;
    if (!option_terse_get())
    {
	usage_col = colp->create(left, left + USAGE_WIDTH, "Type\n-------");
	left += USAGE_WIDTH + 1;

	attr_col_stp = symtab_alloc(5);
	for (j = 0; ; ++j)
	{
	    fstate_src_ty   *src_data;
	    size_t          k;

	    src_data = pp->file_nth(j, view_path_simple);
	    if (!src_data)
		break;
	    if (!src_data->attribute)
		continue;
	    if (src_data->deleted_by && !option_verbose_get())
		continue;
	    if
	    (
		cp
	    &&
		change_file_find(cp, src_data->file_name, view_path_first)
	    )
		continue;
	    for (k = 0; k < src_data->attribute->length; ++k)
	    {
		attributes_ty *ap = src_data->attribute->list[k];
		if (ael_attribute_listable(ap))
		{
		    string_ty *lc_name = str_downcase(ap->name);
		    void *p = symtab_query(attr_col_stp, lc_name);
		    if (!p)
		    {
			string_ty       *s;
			output_ty       *op;

			s = ael_build_header(ap->name);
			op = colp->create(left, left + ATTR_WIDTH, s->str_text);
			str_free(s);
			symtab_assign(attr_col_stp, lc_name, op);
			left += ATTR_WIDTH + 1;
		    }
		    str_free(lc_name);
		}
	    }
	}

	edit_col = colp->create(left, left + EDIT_WIDTH, "Edit\n-------");
	left += EDIT_WIDTH + 1;
    }
    file_name_col = colp->create(left, 0, "File Name\n-----------");

    //
    // list the project's files
    //
    for (j = 0;; ++j)
    {
	fstate_src_ty	*src_data;

	src_data = pp->file_nth(j, view_path_simple);
	if (!src_data)
	    break;
	if (src_data->deleted_by && !option_verbose_get())
	    continue;
	if (cp && change_file_find(cp, src_data->file_name, view_path_first))
	    continue;
	if (option_terse_get())
	{
	    if (src_data->deleted_by)
		continue;
	}
	else
	{
	    usage_col->fputs(file_usage_ename(src_data->usage));
	    list_format_edit_number(edit_col, src_data);
	    if
	    (
		!pp->is_a_trunk()
	    &&
		change_is_a_branch(pp->change_get())
	    &&
		!change_file_up_to_date(pp->parent_get(), src_data)
	    )
	    {
		fstate_src_ty	*psrc_data;

		psrc_data =
		    project_file_find
		    (
			pp->parent_get(),
			src_data->file_name,
			view_path_extreme
		    );
		if (psrc_data && psrc_data->edit)
		{
		    assert(psrc_data->edit->revision);
		    edit_col->fprintf
		    (
			" (%s)",
			psrc_data->edit->revision->str_text
		    );
		}
	    }
	}
	assert(src_data->file_name);
	file_name_col->fputs(src_data->file_name);
	if (src_data->about_to_be_created_by)
	{
	    file_name_col->end_of_line();
	    file_name_col->fprintf
	    (
		"About to be created by change %ld.",
		magic_zero_decode(src_data->about_to_be_created_by)
	    );
	}
	if (src_data->deleted_by)
	{
	    file_name_col->end_of_line();
	    file_name_col->fprintf
	    (
		"Deleted by change %ld.",
		magic_zero_decode(src_data->deleted_by)
	    );
	}
	if (src_data->locked_by)
	{
	    file_name_col->end_of_line();
	    file_name_col->fprintf
	    (
		"Locked by change %ld.",
		magic_zero_decode(src_data->locked_by)
	    );
	}
	if (src_data->about_to_be_copied_by)
	{
	    file_name_col->end_of_line();
	    file_name_col->fprintf
	    (
		"About to be copied by change %ld.",
		magic_zero_decode(src_data->about_to_be_copied_by)
	    );
	}
	if (src_data->move)
	{
	    switch (src_data->action)
	    {
	    case file_action_create:
		file_name_col->end_of_line();
		file_name_col->fputs("Moved from ");
		file_name_col->fputs(src_data->move->str_text);
		break;

	    case file_action_remove:
		file_name_col->end_of_line();
		file_name_col->fputs("Moved to ");
		file_name_col->fputs(src_data->move->str_text);
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
		attributes_ty *ap = src_data->attribute->list[k];
		assert(ap);
		assert(ap->name);
		assert(ap->value);
		if (ap->name && ap->value)
		{
		    string_ty *lc_name = str_downcase(ap->name);
		    output_ty *op =
			(output_ty *)symtab_query(attr_col_stp, lc_name);
		    if (op)
			op->fputs(ap->value);
		    str_free(lc_name);
		}
	    }
	}
	colp->eoln();
    }

    //
    // clean up and go home
    //
    if (attr_col_stp)
	symtab_free(attr_col_stp);
    delete colp;
    project_free(pp);
    if (cp)
	change_free(cp);
    trace(("}\n"));
}
