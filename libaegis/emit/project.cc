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

#include <libaegis/change.h>
#include <libaegis/emit/project.h>
#include <libaegis/http.h>
#include <libaegis/project.h>
#include <libaegis/zero.h>


static void
emit_project_name(project_ty *pp)
{
    if (!pp->is_a_trunk())
    {
	change::pointer cp = pp->change_get();
	emit_project_name(pp->parent_get());
	// the newline is so that the lines don't exceed 510 characters
	printf(".<a\nhref=\"%s/", http_script_name());
	html_escape_string(project_name_get(pp));
	printf("/?menu\">%ld</a>", magic_zero_decode(cp->number));
    }
    else
    {
	printf("<a href=\"%s/", http_script_name());
	html_escape_string(project_name_get(pp));
	printf("/?menu\">");
	html_encode_string(project_name_get(pp));
	printf("</a>");
    }
}


void
emit_project(project_ty *pp)
{
    printf("<a href=\"%s\">Project</a>\n", http_script_name());
    printf("&ldquo;");
    emit_project_name(pp);
    printf("&rdquo;");
}


static void
emit_project_name_but1(project_ty *pp)
{
    if (!pp->is_a_trunk())
    {
	change::pointer cp = pp->change_get();
	emit_project_name(pp->parent_get());
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
    printf("&ldquo;");
    emit_project_name_but1(pp);
    printf("&rdquo;");
}
