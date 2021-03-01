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
// MANIFEST: functions to manipulate projects
//

#include <ac/stdio.h>

#include <change.h>
#include <emit/project.h>
#include <http.h>
#include <project.h>
#include <zero.h>


static void
emit_project_name(project_ty *pp)
{
    if (pp->parent)
    {
	change_ty       *cp;

	cp = project_change_get(pp);
	emit_project_name(pp->parent);
	// the newline is so that the lines don't exceed 510 characters
	printf(".<a\nhref=\"%s/", http_script_name());
	html_escape_string(project_name_get(pp));
	printf("/@@menu\">%ld</a>", magic_zero_decode(cp->number));
    }
    else
    {
	printf("<a href=\"%s/", http_script_name());
	html_escape_string(project_name_get(pp));
	printf("/@@menu\">");
	html_encode_string(project_name_get(pp));
	printf("</a>");
    }
}


void
emit_project(project_ty *pp)
{
    printf("<a href=\"%s\">Project</a>\n", http_script_name());
    printf("\"");
    emit_project_name(pp);
    printf("\"");
}


static void
emit_project_name_but1(project_ty *pp)
{
    if (pp->parent)
    {
	change_ty       *cp;

	cp = project_change_get(pp);
	emit_project_name(pp->parent);
	printf(".%ld", magic_zero_decode(cp->number));
    }
    else
    {
	html_encode_string(project_name_get(pp));
    }
}


void
emit_project_but1(project_ty *pp)
{
    printf("<a href=\"%s\">Project</a>\n", http_script_name());
    printf("\"");
    emit_project_name_but1(pp);
    printf("\"");
}
