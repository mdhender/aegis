//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: implementation of the change_functor_inventory class
//

#pragma implementation "change_functor_inventory"

#include <ac/stdio.h>

#include <change.h>
#include <change/branch.h>
#include <change/functor/inventory.h>
#include <emit/project.h>
#include <http.h>
#include <project.h>


change_functor_inventory::change_functor_inventory(project_ty *arg) :
    pp(arg),
    num(0)
{
    html_header(pp, 0);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(",\nChange Set Inventory</title></head><body>\n");
    html_header_ps(pp, 0);
    printf("<h1 align=center>");
    emit_project(pp);
    printf(",<br>\nChange Set Inventory</h1>\n");

    //
    // list the project's changes
    //
    printf("<div class=\"information\"><table align=\"center\">\n");
    printf("<tr class=\"even-group\"><th>Change</th><th>UUID</th></tr>\n");
}


void
change_functor_inventory::operator()(change_ty *cp)
{
    cstate_ty *cstate_data = change_cstate_get(cp);
    if (!cstate_data->uuid)
	return;
    printf("<tr class=\"%s-group\">", (((num++ / 3) & 1) ? "even" : "odd"));
    printf("<td>");
    emit_change_href(cp, "menu");
    html_encode_string(change_version_get(cp));
    printf("</a></td><td><tt>");
    emit_change_href(cp, "aedist");
    html_encode_string(cstate_data->uuid);
    printf("</a></tt></td></tr>\n");
}


change_functor_inventory::~change_functor_inventory()
{
    printf("<tr class=\"even-group\"><td colspan=2>\n");
    printf("Listed %lu change sets.</td></tr>\n", (unsigned long)num);
    printf("</table></div>\n");

    printf("<hr>\n");
    printf("A similar report may be obtained from the command line, with\n");
    printf("<blockquote><pre>ael cin -p ");
    html_encode_string(project_name_get(pp));
    printf("</pre></blockquote>\n");

    printf("<hr>\n");
    printf("<p align=\"center\" class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(pp, "menu");
    printf("Project Menu</a>\n");
    printf("]</p>\n");

    html_footer(pp, 0);
}
