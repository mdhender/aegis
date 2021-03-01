/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate aliasess
 */

#include <ael/change/inappropriat.h>
#include <ael/column_width.h>
#include <ael/project/aliases.h>
#include <col.h>
#include <error.h> /* for assert */
#include <gonzo.h>
#include <option.h>
#include <output.h>
#include <str_list.h>
#include <trace.h>


void
list_project_aliases(string_ty *project_name, long change_number,
    string_list_ty *args)
{
    string_list_ty  name;
    output_ty       *name_col = 0;
    output_ty       *desc_col = 0;
    int             j;
    col_ty          *colp;
    int             nprinted = 0;

    trace(("list_project_aliases()\n{\n"));
    if (change_number)
	list_change_inappropriate();

    /*
     * list the projects
     */
    gonzo_alias_list(&name);

    /*
     * create the columns
     */
    colp = col_open((string_ty *)0);
    col_title(colp, "List of Project Aliases", (char *)0);

    name_col = col_create(colp, 0, PROJECT_WIDTH, "Alias\n---------");

    if (!option_terse_get())
    {
	desc_col =
	    col_create ( colp, PROJECT_WIDTH + 1, 0, "Project\n-----------");
    }

    /*
     * list each alias
     */
    nprinted = 0;
    for (j = 0; j < name.nstrings; ++j)
    {
	if (project_name)
	{
	    string_ty       *other;

	    other = gonzo_alias_to_actual(name.string[j]);
	    assert(other);
	    if
	    (
		other
	    &&
		(
		    str_equal(other, project_name)
		||
		    str_equal(name.string[j], project_name)
		)
	    )
	    {
		output_put_str(name_col, name.string[j]);
		if (desc_col)
	    	    output_put_str(desc_col, other);
		col_eoln(colp);
		++nprinted;
	    }
	}
	else
	{
	    output_put_str(name_col, name.string[j]);

	    if (desc_col)
	    {
		string_ty       *other;

		other = gonzo_alias_to_actual(name.string[j]);
		assert(other);
		if (other)
	    	    output_put_str(desc_col, other);
	    }
	    col_eoln(colp);
	    ++nprinted;
	}
    }
    if (option_verbose_get() && !nprinted)
    {
	output_ty       *fp;

	output_delete(name_col);
	if (desc_col)
		output_delete(desc_col);
	fp = col_create(colp, 0, 0, (const char *)0);
	output_fputs(fp, "No project aliases.\n");
	col_eoln(colp);
    }

    /*
     * clean up and go home
     */
    col_close(colp);
    trace(("}\n"));
}
