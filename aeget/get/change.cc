//
//	aegis - project change supervisor
//	Copyright (C) 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate changes
//

#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <libaegis/change.h>
#include <aeget/get/change.h>
#include <aeget/get/change/aedist.h>
#include <aeget/get/change/aepatch.h>
#include <aeget/get/change/aerevml.h>
#include <aeget/get/change/aetar.h>
#include <aeget/get/change/download.h>
#include <aeget/get/change/file_invento.h>
#include <aeget/get/change/files.h>
#include <aeget/get/change/history.h>
#include <aeget/get/change/menu.h>
#include <aeget/get/file.h>
#include <aeget/get/project.h>
#include <aeget/http.h>
#include <libaegis/project.h>
#include <common/str_list.h>


static void
project_handoff(change_ty *cp, string_ty *filename, string_list_ty *modifier)
{
    get_project(cp->pp, filename, modifier);
}


struct table_ty
{
    const char *name;
    void (*action)(change_ty *, string_ty *, string_list_ty *);
    int hide_me;
};

static const table_ty table[] =
{
    //
    // To remove ambiguity at the root level, sometimes you have
    // to say "?file+history" to distinguish the request from
    // "?project+history"
    //
    { "project", project_handoff, 1 },
    { "file", get_file, 1 },
    { "change", get_change, 1 },

    { "aedist", get_change_aedist },
    { "aepatch", get_change_aepatch },
    { "aerevml", get_change_aerevml },
    { "aetar", get_change_aetar },
    { "download", get_change_download },
    { "files", get_change_files },
    { "file-inventory", get_change_file_inventory },
    { "history", get_change_history },
    { "menu", get_change_menu },
};


static void
whine(change_ty *cp, string_list_ty *modifier)
{
    html_header(0, cp);
    printf("<title>Project ");
    html_encode_string(project_name_get(cp->pp));
    printf(", Change %ld", magic_zero_decode(cp->number));
    printf(",\nPuzzlement</title></head><body>\n");
    html_header_ps(0, cp);
    printf("<h1 align=center>");
    emit_change(cp);
    printf(",<br>\nPuzzlement</h1>\n");
    printf("Change information selector ");
    if (modifier->nstrings)
    {
	printf("&ldquo;<tt>");
	html_encode_string(modifier->string[0]);
	printf("</tt>&rdquo; ");
    }
    printf("not recognised.\n");
    printf("Please select one of the following:\n");
    printf("<ul>\n");
    for (const table_ty *tp = table; tp < ENDOF(table); ++tp)
    {
	if (tp->hide_me)
	    continue;
	string_ty *s = str_format("change+%s", tp->name);
	printf("<li>");
	emit_change_href(cp, s->str_text);
	printf("%s</a>\n", tp->name);
	str_free(s);
    }
    printf("</ul>\n");

    printf("<hr>\n");
    printf("<p align=\"center\" class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(cp->pp, "menu");
    printf("Project Menu</a> |\n");
    emit_change_href(cp, "menu");
    printf("Change List</a> |\n");
    emit_change_href(cp, "menu");
    printf("Change Menu</a>\n");
    printf("]</p>\n");

    html_footer(0, cp);
}


void
get_change(change_ty *cp, string_ty *filename, string_list_ty *modifier)
{
    //
    // Look for the change modifier to report.
    //
    if (modifier->nstrings && 0 == strcmp(filename->str_text, "."))
    {
	for (const table_ty *tp = table; tp < ENDOF(table); ++tp)
	{
	    if (modifier_test_and_clear(modifier, tp->name))
	    {
		tp->action(cp, filename, modifier);
		return;
	    }
	}

	//
	// Whine about bogus requests.
	//
	whine(cp, modifier);
	return;
    }

    //
    // The default is to look for the file contents.
    //
    get_file(cp, filename, modifier);
}
