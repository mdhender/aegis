//
//	aegis - project change supervisor
//	Copyright (C) 2004-2007 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <common/uuidentifier.h>
#include <common/version_stmp.h>
#include <libaegis/attribute.h>
#include <libaegis/change/attributes.h>
#include <libaegis/change/branch.h>
#include <libaegis/change.h>
#include <libaegis/project.h>

#include <aeget/change/functor/inventory.h>
#include <aeget/emit/project.h>
#include <aeget/http.h>


change_functor_inventory::change_functor_inventory(bool arg1, project_ty *arg2,
	bool arg3) :
    change_functor(arg1),
    pp(arg2),
    include_original_uuid(arg3),
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
    printf("<tr class=\"even-group\"><th>Change</th><th>UUID</th>");
    printf("<th>Description</th></tr>\n");
}


static int
max_printable(const char *s)
{
    int len = 0;
    for (;;)
    {
	unsigned char c = *s++;
	if (!c || !isprint(c))
	    break;
	++len;
	if (len >= 200)
	    break;
    }
    return len;
}


void
change_functor_inventory::print_one_line(change::pointer cp, string_ty *uuid)
{
    //
    // If the change's aeget:inventory:hide attribute is true, do not
    // emit this change set into the inventory listing used by
    // aedist --missing, et al.
    //
    if (change_attributes_find_boolean(cp, "aeget:inventory:hide"))
	return;

    printf("<tr class=\"%s-group\">", (((num++ / 3) & 1) ? "even" : "odd"));
    printf("<td>");
    emit_change_href(cp, "menu");
    html_encode_string(change_version_get(cp));
    printf("</a></td><td><tt>");
    emit_change_href
    (
	cp,
	nstring::format("aedist+compat=%s", version_stamp()).c_str()
    );
    html_encode_string(uuid);
    printf("</a></tt></td><td>");

    //
    // We are deliberately not using emit_change_brief_description to
    // ensure that this row only ever takes one line, and contains no
    // html anchor.  This makes it simple to parse in aedist --missing
    //
    string_ty *s = change_brief_description_get(cp);
    s = str_n_from_c(s->str_text, max_printable(s->str_text));
    html_encode_string(s);
    str_free(s);
    printf("</td></tr>\n");
}


void
change_functor_inventory::operator()(change::pointer cp)
{
    cstate_ty *cstate_data = cp->cstate_get();
    if (cstate_data->uuid)
	print_one_line(cp, cstate_data->uuid);
    if (include_original_uuid && cstate_data->attribute)
    {
	for (size_t j = 0; j < cstate_data->attribute->length; ++j)
	{
	    attributes_ty *ap = cstate_data->attribute->list[j];
	    assert(ap);
	    if
    	    (
		ap->name
	    &&
		0 == strcasecmp(ORIGINAL_UUID, ap->name->str_text)
	    &&
		ap->value
	    &&
		// users can edit, make sure is OK
		universal_unique_identifier_valid(ap->value)
	    )
	    {
		print_one_line(cp, ap->value);
	    }
	}
    }
}


change_functor_inventory::~change_functor_inventory()
{
    printf("<tr class=\"even-group\"><td colspan=3>\n");
    printf("Listed %lu change sets.</td></tr>\n", (unsigned long)num);
    printf("</table></div>\n");

    printf("<p>There is also a page containing this list ");
    if (include_original_uuid)
    {
	emit_project_href(pp, "inventory");
	printf("without");
    }
    else
    {
	emit_project_href(pp, "inventory+all");
	printf("with");
    }
    printf("</a> the original-uuid attributes.</p>");

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
