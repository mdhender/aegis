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
//	along with this program.  If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/http.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>

#include <aeget/get/project/list.h>


void
get_project_list(void)
{
    string_list_ty  name;
    size_t          j;
    int             num;

    trace(("get_project_list()\n{\n"));

    //
    // get the projects
    //
    project_list_get(&name);

    html_header(0, 0);
    printf("<title>Project List</title></head><body>\n");
    html_header_ps(0, 0);
    printf("<h1 align=center>Project List</h1>\n");
    printf("<div class=\"information\">\n");
    printf("<table align=center>\n");
    printf("<tr class=\"even-group\"><th>Project</th>");
    printf("<th>Description</th></tr>\n");

    //
    // list each project
    //
    num = 0;
    for (j = 0; j < name.nstrings; ++j)
    {
	project_ty	*pp;
	int		err;

	pp = project_alloc(name.string[j]);
	pp->bind_existing();

	err = project_is_readable(pp);
	if (err)
	    continue;

	printf("<tr class=\"%s-group\">", ((num / 3) & 1) ? "even" : "odd");
	printf("<td valign=top>\n");
	emit_project_href(pp, "menu");
	html_encode_string(project_name_get(pp));
	printf("</a>\n</td><td valign=top>\n");
	html_encode_string(project_description_get(pp));
	project_free(pp);
	printf("\n</td></tr>\n");
	++num;
    }

    //
    // clean up and go home
    //
    printf("<tr class=\"even-group\"><td colspan=2>");
    printf("Listed %d projects.</td></tr>\n", num);
    printf("</table></div>\n");

    printf("<hr>\n");
    printf("A similar report may be obtained from the command line, with\n");
    printf("<blockquote><pre>ael p</pre></blockquote>\n");

    html_footer(0, 0);
    trace(("}\n"));
}
