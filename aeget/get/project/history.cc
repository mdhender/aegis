//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller;
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
// MANIFEST: implementation of the get_project_history class
//

#include <common/ac/stdio.h>

#include <common/error.h> // for assert
#include <libaegis/change/branch.h>
#include <libaegis/change.h>
#include <libaegis/cstate.h>
#include <libaegis/project.h>

#include <aeget/emit/brief_descri.h>
#include <aeget/emit/project.h>
#include <aeget/get/project/history.h>
#include <aeget/http.h>


void
get_project_history(project_ty *pp, string_ty *fn, string_list_ty *modifier_p)
{
    //
    // Emit the title portion.
    //
    html_header(pp, 0);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(" History</title></head><body>\n");
    html_header_ps(pp, 0);

    //
    // Emit the heading portion.
    //
    printf("<h1 align=center>\n");
    emit_project(pp);
    printf(",<br>\nHistory</h1>\n");

    //
    // Emit the column headings.
    //
    printf("<div class=\"information\">\n");
    printf("<table align=center>\n");
    printf("<tr class=\"even-group\"><th>Delta</th><th>When</th>");
    printf("<th>Change</th><th>Description</th><th>&nbsp;</th></tr>\n");

    //
    // Emit the project history.
    //
    change_ty *pcp = pp->change_get();
    cstate_ty *proj_cstate_data = change_cstate_get(pcp);
    assert(proj_cstate_data->branch);
    if (!proj_cstate_data->branch->history)
    {
	proj_cstate_data->branch->history =
	    (cstate_branch_history_list_ty *)cstate_branch_history_type.alloc();
    }
    long rownum = 0;
    for (size_t j = 0; j < proj_cstate_data->branch->history->length; ++j)
    {
	cstate_branch_history_ty *hp =
	    proj_cstate_data->branch->history->list[j];
	change_ty *cp = change_alloc(pp, hp->change_number);
	change_bind_existing(cp);
	assert(change_is_completed(cp));

	const char *html_class = (((rownum++ / 3) & 1) ?  "even" : "odd");
	printf("<tr class=\"%s-group\">", html_class);
	printf("<td valign=top align=right>");
	emit_change_href(cp, "menu");
	printf("%ld</a></td>\n", hp->delta_number);

	time_t when = change_completion_timestamp(cp);
	struct tm *tmp = localtime(&when);
	char buffer[100];
	strftime(buffer, sizeof(buffer), "%a %b %e\n%H:%M:%S %Y", tmp);
	printf("<td valign=top>%s</td>\n", buffer);

	printf("<td valign=top align=right>");
	emit_change_href(cp, "menu");
	printf("%ld</a></td>\n", hp->change_number);

	printf("</td><td valign=top>");
	emit_change_brief_description(cp);
	printf("</td>\n");

	printf("<td valign=top>");
	emit_change_href(cp, "download");
	printf("Download</a></td></tr>\n");
    }

    //
    // Emit the summary row.
    //
    printf("<tr class=\"even-group\"><td colspan=5>");
    printf("Listed %ld completed changes.", rownum);
    printf("</td></tr>\n");
    printf("</table>\n");
    printf("</div>\n");

    //
    // Emit the command line equivalent.
    //
    printf("<hr>");
    printf("<div class=\"report-cmd\">");
    printf("A similar report may be obtained from the command line,\n");
    printf("with one of\n<blockquote><pre>ael proj_history -p ");
    html_encode_string(project_name_get(pp));
    printf("\naer proj_history -p ");
    html_encode_string(project_name_get(pp));
    printf("</pre></blockquote></div>\n");

    //
    // Emit the footer portion.
    //
    printf("<hr>\n");
    printf("<p align=center class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(pp, "menu");
    printf("Project Menu</a>\n]</p>\n");

    html_footer(pp, 0);
}
