//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate lists
//

#include <ac/string.h>

#include <ael/change/inappropriat.h>
#include <ael/project/inappropriat.h>
#include <gonzo.h>
#include <io.h>
#include <output.h>
#include <project.h>
#include <project/history.h>
#include <str_list.h>
#include <trace.h>
#include <xml/project/list.h>


void
xml_project_list(struct string_ty *project_name, long change_number,
    struct output_ty *op)
{
    string_list_ty  name;
    size_t	j;

    trace(("xml_project_list()\n{\n"));
    if (project_name)
	list_project_inappropriate();
    if (change_number)
	list_change_inappropriate();

    //
    // list the projects
    //
    project_list_get(&name);

    //
    // list each project
    //
    output_fprintf(op, "<gstate>\n<where>\n");
    for (j = 0; j < name.nstrings; ++j)
    {
	project_ty	*pp;
	int		err;

	output_fprintf(op, "<gstate_where>\n");
	pp = project_alloc(name.string[j]);
	project_bind_existing(pp);
	string_write_xml(op, "name", project_name_get(pp));

	err = project_is_readable(pp);

	if (err)
	{
	    output_fprintf(op, "<error>%s</error>\n", strerror(err));
	}
	else
	{
	    string_ty       *top;

	    string_write_xml(op, "description", project_description_get(pp));

	    //
	    // The development directory of the project change is the
	    // one which contains the trunk or branch baseline.
	    //
	    top = project_top_path_get(pp, 0);
	    string_write_xml(op, "directory", top);
	}
	project_free(pp);
	output_fprintf(op, "</gstate_where>\n");
    }
    output_fprintf(op, "</where>\n</gstate>\n");
    trace(("}\n"));
}
