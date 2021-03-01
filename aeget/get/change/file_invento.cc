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
// MANIFEST: implementation of the get_change_file_invento class
//

#include <ac/stdio.h>

#include <change.h>
#include <change/file.h>
#include <get/change/file_invento.h>
#include <http.h>
#include <project/file.h>


void
get_change_file_inventory(change_ty *cp, string_ty *filename, string_list_ty *)
{
    html_header(0, cp);
    printf("<title>Project\n");
    html_encode_string(project_name_get(cp->pp));
    if (!cp->bogus)
	printf(", Change %ld", magic_zero_decode(cp->number));
    printf(", File Inventory\n");
    printf("</title></head><body>\n");
    html_header_ps(0, cp);
    printf("<h1 align=center>\n");
    emit_change(cp);
    printf(",<br>\nFile Inventory</h1>\n");
    printf("<div class=\"information\">\n");

    printf("<table align=center>\n");
    printf("<tr class=\"even-group\"><th>File Name</th><th>UUID</th></tr>\n");

    size_t j;
    for (j = 0; ; ++j)
    {
	fstate_src_ty *src = 0;
	if (cp->bogus)
	    src = project_file_nth(cp->pp, j, view_path_extreme);
	else
	    src = change_file_nth(cp, j, view_path_first);
	if (!src)
	    break;

	printf("<tr class=\"%s-group\">", (j % 6 < 3 ? "odd" : "even"));
	printf("<td valign=top>\n");
	emit_file_href(cp, src->file_name, "menu");
	printf("<span class=\"filename\">");
	html_encode_string(src->file_name);
	printf("</span></a></td>\n<td>");
	if (src->uuid)
	{
	    printf("<tt>");
	    html_encode_string(src->uuid);
	    printf("</tt>");
	}
	printf("</td></tr>\n");
    }
    printf("<tr class=\"even-group\"><td colspan=4>\n");
    printf("Listed %ld files.\n", (long)j);
    printf("</td></tr>\n");
    printf("</table>\n");
    printf("</div>\n");

    printf("<hr>A similar report may be obtained from the command line,\n");
    printf("with the following command:\n<blockquote><pre>");
    if (!cp->bogus)
    {
	printf("ael cfin -p ");
	html_encode_string(project_name_get(cp->pp));
	printf(" -c %ld", magic_zero_decode(cp->number));
    }
    else
    {
	printf("ael pfin -p ");
	html_encode_string(project_name_get(cp->pp));
    }
    printf("</pre></blockquote>\n");

    printf("<hr>\n");
    printf("<p align=center class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(cp->pp, "menu");
    printf("Project Menu</a> |\n");
    emit_project_href(cp->pp, "changes");
    printf("Change List</a>\n");
    if (!cp->bogus)
    {
	printf("| ");
	emit_change_href(cp, "menu");
	printf("Change Menu</a>\n");
    }
    printf("]</p>\n");

    html_footer(0, cp);
}
