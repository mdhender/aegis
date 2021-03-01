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
#include <ac/string.h>

#include <emit/project.h>
#include <get/change.h>
#include <get/change/list.h>
#include <get/change/inventory.h>
#include <get/project.h>
#include <get/project/aedist.h>
#include <get/project/change_cause.h>
#include <get/project/file.h>
#include <get/project/file_invento.h>
#include <get/project/files.h>
#include <get/project/history.h>
#include <get/project/integr_histo.h>
#include <get/project/menu.h>
#include <get/project/progress.h>
#include <get/project/staff.h>
#include <get/project/statistics.h>
#include <http.h>
#include <project.h>
#include <str_list.h>


static void
change_handoff(project_ty *pp, string_ty *filename, string_list_ty *modifier)
{
    get_change(project_change_get(pp), filename, modifier);
}


struct table_ty
{
    const char *name;
    void (*action)(project_ty *, string_ty *, string_list_ty *);
    int hide_me;
};


static const table_ty table[] =
{
    //
    // To remove ambiguity at the root level, sometimes you have
    // to say "?file+history" to distinguish the request from
    // "?project+history"
    //
    { "file", get_project_file, 1 },
    { "change", change_handoff, 1 },
    { "project", get_project, 1 },

    { "aedist", get_project_aedist },
    { "changes", get_change_list },
    { "change-cause", get_project_change_cause },
    { "history", get_project_history },
    { "integration-histogram", get_project_integration_histogram },
    { "inventory", get_change_inventory },
    { "file-inventory", get_project_file_inventory },
    { "files", get_project_files },
    { "menu", get_project_menu },
    { "progress", get_project_progress },
    { "staff", get_project_staff },
    { "statistics", get_project_statistics },
};


static void
whine(project_ty *pp, string_list_ty *modifier)
{
    html_header(pp, 0);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(",\nPuzzlement</title></head><body>\n");
    html_header_ps(pp, 0);
    printf("<h1 align=center>");
    emit_project(pp);
    printf(",<br>\nPuzzlement</h1>\n");
    printf("Project information selector ");
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
	string_ty *s = str_format("project+%s", tp->name);
	printf("<li>");
	emit_project_href(pp, "%s", s->str_text);
	printf("%s</a>\n", tp->name);
	str_free(s);
    }
    printf("</ul>\n");

    printf("<hr>\n");
    printf("<p align=\"center\" class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(pp, "menu");
    printf("Project Menu</a>\n");
    printf("]</p>\n");

    html_footer(pp, 0);
}


void
get_project(project_ty *pp, string_ty *filename, string_list_ty *modifier)
{
    fprintf(stderr, "%s: %d: get_project\n", __FILE__, __LINE__);
    //
    // Look for the project modifier to report.
    //
    if (modifier->nstrings && 0 == strcmp(filename->str_text, "."))
    {
	for (const table_ty *tp = table; tp < ENDOF(table); ++tp)
	{
	    if (modifier_test_and_clear(modifier, tp->name))
	    {
    fprintf(stderr, "%s: %d: get_project %s\n", __FILE__, __LINE__, tp->name);
		tp->action(pp, filename, modifier);
		return;
	    }
	}

	//
	// Whine about bogus requests.
	//
	whine(pp, modifier);
	return;
    }

    //
    // The default is to look for the file contents.
    //
    get_project_file(pp, filename, modifier);
}
