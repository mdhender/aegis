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
 * MANIFEST: functions to manipulate aliasess
 */

#include <ael/change/inappropriat.h>
#include <ael/column_width.h>
#include <ael/project/aliases.h>
#include <ael/project/inappropriat.h>
#include <col.h>
#include <error.h> /* for assert */
#include <gonzo.h>
#include <option.h>
#include <str_list.h>
#include <trace.h>


void
list_project_aliases(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	string_list_ty	name;
	int		name_col = 0;
	int		desc_col = 0;
	int		j;

	trace(("list_project_aliases()\n{\n"));
	if (project_name)
		list_project_inappropriate();
	if (change_number)
		list_change_inappropriate();

	/*
	 * list the projects
	 */
	gonzo_alias_list(&name);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	col_title("List of Project Aliases", (char *)0);

	name_col = col_create(0, PROJECT_WIDTH);
	col_heading(name_col, "Alias\n---------");

	if (!option_terse_get())
	{
		desc_col = col_create(PROJECT_WIDTH + 1, 0);
		col_heading(desc_col, "Project\n-----------");
	}

	/*
	 * list each alias
	 */
	for (j = 0; j < name.nstrings; ++j)
	{
		col_puts(name_col, name.string[j]->str_text);

		if (!option_terse_get())
		{
			string_ty	*other;

			other = gonzo_alias_to_actual(name.string[j]);
			assert(other);
			if (other)
				col_puts(desc_col, other->str_text);
		}
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	trace(("}\n"));
}
