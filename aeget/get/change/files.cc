//
//	aegis - project change supervisor
//	Copyright (C) 2003-2008 Peter Miller
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

#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <common/str_list.h>
#include <libaegis/ael/attribu_list.h>
#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/emit/edit_number.h>
#include <libaegis/http.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>

#include <aeget/get/change/files.h>


void
get_change_files(change::pointer cp, string_ty *, string_list_ty *)
{
    string_list_ty  attr_name;
    for (size_t j = 0; ; ++j)
    {
	fstate_src_ty   *src;

	if (cp->bogus)
	    src = cp->pp->file_nth(j, view_path_simple);
	else
	    src = change_file_nth(cp, j, view_path_first);
	if (!src)
	    break;
	if (src->attribute)
	{
	    for (size_t k = 0; k < src->attribute->length; ++k)
	    {
		attributes_ty   *ap;

		ap = src->attribute->list[k];
		if
		(
		    ael_attribute_listable(ap)
		&&
		    !attr_name.member_nocase(ap->name)
		)
		{
		    attr_name.push_back(ap->name);
		}
	    }
	}
    }
    attr_name.sort_nocase();

    html_header(0, cp);
    printf("<title>Project\n");
    html_encode_string(project_name_get(cp->pp));
    if (!cp->bogus)
	printf(", Change %ld", magic_zero_decode(cp->number));
    printf(", Files\n");
    printf("</title></head><body>\n");
    html_header_ps(0, cp);
    printf("<h1 align=center>\n");
    emit_change(cp);
    printf(",<br>\nList of Files</h1>\n");
    printf("<div class=\"information\">\n");

    printf("<table align=center>\n");
    printf("<tr class=\"even-group\"><th>Type</th><th>Action</th>");
    printf("<th>Edit</th>\n");
    for (size_t j2 = 0; j2 < attr_name.nstrings; ++j2)
    {
	printf("<th>");
	html_encode_string(attr_name.string[j2]);
	printf("</th>\n");
    }
    printf("<th>File Name</th></tr>\n");

    size_t file_num;
    for (file_num = 0; ; ++file_num)
    {
	fstate_src_ty   *src;

	if (cp->bogus)
	    src = cp->pp->file_nth(file_num, view_path_simple);
	else
	    src = change_file_nth(cp, file_num, view_path_first);
	if (!src)
	    break;

	printf
	(
	    "<tr class=\"%s-group\">\n",
	    (file_num % 6 < 3 ? "odd" : "even")
	);

	printf("<td valign=top>%s</td>\n", file_usage_ename(src->usage));
	printf("<td valign=top>%s</td>\n", file_action_ename(src->action));
	printf("<td valign=top align=right>\n");
	emit_edit_number(cp, src, 0);
	printf("</td>\n");
	for (size_t k = 0; k < attr_name.nstrings; ++k)
	{
	    const char *aname = attr_name.string[k]->str_text;
	    printf("<td valign=top>");
	    attributes_ty *ap = attributes_list_find(src->attribute, aname);
	    if (ap)
		html_encode_string(ap->value);
	    printf("</td>\n");
	}
	printf("<td valign=top>\n");
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
    printf("Listed %ld files.\n", (long)file_num);
    printf("</td></tr>\n");
    printf("</table>\n");

    if (!cp->bogus)
    {
	cstate_ty *cstate_data = cp->cstate_get();
	if (cstate_data->state >= cstate_state_being_developed)
	{
	    printf("<p>\n");
	    printf("You may also be interested in the ");
	    emit_change_href(cp, "file+history");
	    printf("history</a> listing.\n");
	}
    }
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

    html_footer(0, cp);
}
