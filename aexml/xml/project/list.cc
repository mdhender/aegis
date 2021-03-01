//
//	aegis - project change supervisor
//	Copyright (C) 2003-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
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

#include <common/ac/string.h>

#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/project/inappropriat.h>
#include <libaegis/gonzo.h>
#include <libaegis/io.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <aexml/xml/project/list.h>


void
xml_project_list(struct string_ty *project_name, long change_number,
    output::pointer op)
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
    op->fputs("<gstate>\n<where>\n");
    for (j = 0; j < name.nstrings; ++j)
    {
	project_ty	*pp;
	int		err;

	op->fputs("<gstate_where>\n");
	pp = project_alloc(name.string[j]);
	pp->bind_existing();
	string_write_xml(op, "name", project_name_get(pp));

	err = project_is_readable(pp);

	if (err)
	{
	    op->fprintf("<error>%s</error>\n", strerror(err));
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
	op->fputs("</gstate_where>\n");
    }
    op->fputs("</where>\n</gstate>\n");
    trace(("}\n"));
}
