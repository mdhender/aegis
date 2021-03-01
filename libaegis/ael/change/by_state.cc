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
// MANIFEST: functions to manipulate by_states
//

#include <ael/attribu_list.h>
#include <ael/build_header.h>
#include <ael/change/by_state.h>
#include <ael/column_width.h>
#include <change.h>
#include <col.h>
#include <option.h>
#include <output.h>
#include <project.h>
#include <project/history.h>
#include <symtab.h>
#include <trace.h>
#include <user.h>


static int
single_bit(int n)
{
    int		result;

    // see if no bits are set
    if (!n)
	return -1;

    //
    // see if more than 1 bit is set
    //	(only works on 2s compliment machines)
    //
    if ((n & -n) != n)
	return -1;

    //
    // will need to extend this for 64bit machines,
    // if ever have >32 states
    //
    result = 0;
    if (n & 0xFFFF0000)
	result += 16;
    if (n & 0xFF00FF00)
	result += 8;
    if (n & 0xF0F0F0F0)
	result += 4;
    if (n & 0xCCCCCCCC)
	result += 2;
    if (n & 0xAAAAAAAA)
	result++;
    return result;
}


static void
output_reaper(void *p)
{
    output_ty       *op;

    op = (output_ty *)p;
    output_delete(op);
}


void
list_changes_in_state_mask_by_user(string_ty *project_name, int state_mask,
    string_ty *login)
{
    output_ty       *number_col = 0;
    output_ty       *state_col = 0;
    output_ty       *description_col = 0;
    int             j;
    project_ty      *pp;
    string_ty       *line1;
    string_ty       *line2;
    int             left;
    col_ty	    *colp;
    user_ty         *up;
    symtab_ty       *attr_col_stp;

    //
    // Check that the specified user exists.
    //
    attr_col_stp = 0;
    if (login)
        up = user_symbolic((project_ty *)0, login);
    else
        up = user_executing((project_ty *)0);

    //
    // locate project data
    //
    trace(("list_changes_in_state_mask(state_mask = 0x%X)\n{\n", state_mask));
    if (!project_name)
	project_name = user_default_project_by_user(up);
    else
	project_name = str_copy(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    //
    // create the columns
    //
    colp = col_open((string_ty *)0);
    line1 = str_format("Project \"%s\"", project_name_get(pp)->str_text);
    j = single_bit(state_mask);
    if (j >= 0)
    {
	line2 =
	    str_format
	    (
		"List of Changes %s",
		cstate_state_ename((cstate_state_ty)j)
	    );
    }
    else
    {
	j = single_bit(~state_mask);
	if (j >= 0)
	{
	    line2 =
		str_format
		(
		    "List of Changes not %s",
		    cstate_state_ename((cstate_state_ty)j)
		);
	}
	else
	    line2 = str_from_c("List of Changes");
    }
    col_title(colp, line1->str_text, line2->str_text);
    str_free(line1);
    str_free(line2);

    left = 0;
    number_col = col_create(colp, left, left + CHANGE_WIDTH, "Change\n-------");
    left += CHANGE_WIDTH + 1;

    if (!option_terse_get())
    {
	state_col =
	    col_create(colp, left, left + STATE_WIDTH, "State\n-------");
	left += STATE_WIDTH + 1;

	attr_col_stp = symtab_alloc(5);
	attr_col_stp->reap = output_reaper;
	for (j = 0; ; ++j)
	{
	    cstate_ty       *cstate_data;
	    long		change_number;
	    change_ty	*cp;

	    if (!project_change_nth(pp, j, &change_number))
		break;
	    cp = change_alloc(pp, change_number);
	    change_bind_existing(cp);
	    cstate_data = change_cstate_get(cp);
	    if
	    (
		(state_mask & (1 << cstate_data->state))
	    &&
		//
		// If no user has been specified, we don't care who the
		// owner of the change is.
		//
		(!login || str_equal(login, change_developer_name(cp)))
	    &&
		cstate_data->attribute
	    )
	    {
		size_t          k;

		for (k = 0; k < cstate_data->attribute->length; ++k)
		{
		    attributes_ty   *ap;

		    ap = cstate_data->attribute->list[k];
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
	    change_free(cp);
	}

	description_col =
	    col_create(colp, left, 0, "Description\n-------------");
    }

    //
    // list the project's changes
    //
    for (j = 0; ; ++j)
    {
	cstate_ty       *cstate_data;
	long		change_number;
	change_ty	*cp;

	if (!project_change_nth(pp, j, &change_number))
	    break;
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
	cstate_data = change_cstate_get(cp);
	if
	(
	    (state_mask & (1 << cstate_data->state))
	&&
            //
	    // If no user has been specified, we don't care who the
	    // owner of the change is.
	    //
            (!login || str_equal(login, change_developer_name(cp)))
	)
	{
	    output_fprintf
	    (
		number_col,
		"%4ld",
		magic_zero_decode(change_number)
	    );
	    if (state_col)
	    {
		output_fputs(state_col, cstate_state_ename(cstate_data->state));
		if
		(
		    option_verbose_get()
		&&
		    cstate_data->state == cstate_state_being_developed
		)
		{
		    output_end_of_line(state_col);
		    output_put_str(state_col, change_developer_name(cp));
		}
		if
		(
		    option_verbose_get()
		&&
		    cstate_data->state == cstate_state_being_integrated
		)
		{
		    output_end_of_line(state_col);
		    output_put_str(state_col, change_integrator_name(cp));
		}
	    }
	    if (description_col && cstate_data->brief_description)
	    {
		output_put_str(description_col, cstate_data->brief_description);
	    }
	    if (attr_col_stp && cstate_data->attribute)
	    {
		size_t          k;

		for (k = 0; k < cstate_data->attribute->length; ++k)
		{
		    attributes_ty   *ap;

		    ap = cstate_data->attribute->list[k];
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
	change_free(cp);
    }
    if (attr_col_stp)
	symtab_free(attr_col_stp);

    //
    // clean up and go home
    //
    col_close(colp);
    project_free(pp);
    trace(("}\n"));
}


void
list_changes_in_state_mask(string_ty *project_name, int state_mask)
{
    list_changes_in_state_mask_by_user(project_name, state_mask, 0);
}
