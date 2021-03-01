/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate filess
 */

#include <ac/stdio.h>

#include <change.h>
#include <change/file.h>
#include <emit/edit_number.h>
#include <error.h> /* for assert */
#include <get/change/files.h>
#include <http.h>
#include <project.h>
#include <project/file.h>
#include <str_list.h>


void
get_change_files(change_ty *cp, string_ty *filename, string_list_ty *modifier)
{
    size_t          j;

    html_header(cp->pp);
    printf("<title>Project\n");
    html_encode_string(project_name_get(cp->pp));
    if (!cp->bogus)
	printf(", Change %ld", cp->number);
    printf(", Files\n");
    printf("</title></head>\n<body><h1 align=center>\n");
    emit_change(cp);
    printf(",<br>\nList of Files</h1>\n");
    printf("<div class=\"information\">\n");

    printf("<table align=center>\n");
    printf("<tr class=\"even-group\"><th>Type</th><th>Action</th>");
    printf("<th>Edit</th><th>File Name</th></tr>\n");

    for (j = 0; ; ++j)
    {
	fstate_src_ty   *src;

	if (cp->bogus)
	    src = project_file_nth(cp->pp, j, view_path_simple);
	else
	    src = change_file_nth(cp, j);
	if (!src)
	    break;

	printf("<tr class=\"%s-group\">\n", (j % 6 < 3 ? "odd" : "even"));

	printf("<td valign=top>%s</td>\n", file_usage_ename(src->usage));
	printf("<td valign=top>%s</td>\n", file_action_ename(src->action));
	printf("<td valign=top align=right>\n");
	emit_edit_number(cp, src);
	printf("</td><td valign=top>\n");
	emit_file_href(cp, src->file_name, "menu");
	printf("<span class=\"filename\">");
	html_encode_string(src->file_name);
	printf("</span></a>");
	if (src->deleted_by)
	{
	    printf("<br>\nDeleted by change ");
	    emit_change_href_n(cp->pp, src->deleted_by, "menu");
	    printf("%ld</a>.", src->deleted_by);
	}
	if (src->about_to_be_created_by)
	{
	    printf("<br>\nAbout to be created by change ");
	    emit_change_href_n(cp->pp, src->about_to_be_created_by, "menu");
	    printf("%ld</a>.", src->about_to_be_created_by);
	}
	if (src->locked_by)
	{
	    printf("<br>\nLocked by change ");
	    emit_change_href_n(cp->pp, src->locked_by, "menu");
	    printf("%ld</a>.", src->locked_by);
	}
	if (src->about_to_be_copied_by)
	{
	    printf("<br>\nAbout to be copied by change ");
	    emit_change_href_n(cp->pp, src->about_to_be_copied_by, "menu");
	    printf("%ld</a>.", src->about_to_be_copied_by);
	}
	if (src->move)
	{
	    switch (src->action)
	    {
	    case file_action_create:
		printf("<br>\nMoved from ");
		emit_file_href(cp, src->move, "menu");
		printf("<span class=\"filename\">");
		html_encode_string(src->move);
		printf("</span></a>");
		break;

	    case file_action_remove:
		printf("<br>\nMoved to ");
		emit_file_href(cp, src->move, "menu");
		printf("<span class=\"filename\">");
		html_encode_string(src->move);
		printf("</span></a>");
		break;

	    case file_action_modify:
	    case file_action_insulate:
	    case file_action_transparent:
		assert(0);
		break;
	    }
	}
	printf("</td></tr>\n");
    }
    printf("<tr class=\"even-group\"><td colspan=4>\n");
    printf("Listed %ld files.\n", (long)j);
    printf("</td></tr>\n");
    printf("</table>\n");
    printf("</div>\n");

    printf("<hr>A similar report may be obtained from the command line,\n");
    printf("with one of\n<blockquote><pre>");
    if (!cp->bogus)
    {
	printf("ael cf -p ");
	html_encode_string(project_name_get(cp->pp));
	printf(" -c %ld\n", magic_zero_decode(cp->number));
	printf("aer cf -p ");
	html_encode_string(project_name_get(cp->pp));
	printf(" -c %ld", magic_zero_decode(cp->number));
    }
    else
    {
	printf("ael pf -p ");
	html_encode_string(project_name_get(cp->pp));
	printf("\naer proj_files -p ");
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

    html_footer();
}
