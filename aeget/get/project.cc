//
//      aegis - project change supervisor
//      Copyright (C) 2003-2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program.  If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <common/str_list.h>
#include <libaegis/emit/project.h>
#include <libaegis/http.h>
#include <libaegis/project.h>

#include <aeget/get/change.h>
#include <aeget/get/change/inventory.h>
#include <aeget/get/change/list.h>
#include <aeget/get/project.h>
#include <aeget/get/project/aedist.h>
#include <aeget/get/project/change_cause.h>
#include <aeget/get/project/file.h>
#include <aeget/get/project/file_invento.h>
#include <aeget/get/project/files.h>
#include <aeget/get/project/history.h>
#include <aeget/get/project/integr_histo.h>
#include <aeget/get/project/menu.h>
#include <aeget/get/project/progress.h>
#include <aeget/get/project/staff.h>
#include <aeget/get/project/statistics.h>
#include <aeget/get/rss.h>


static void
change_handoff(project_ty *pp, string_ty *filename, string_list_ty *modifier)
{
    get_change(pp->change_get(), filename, modifier);
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

    { "aedist", get_project_aedist, 0 },
    { "changes", get_change_list, 0 },
    { "change-cause", get_project_change_cause, 0 },
    { "history", get_project_history, 0 },
    { "integration-histogram", get_project_integration_histogram, 0 },
    { "inventory", get_change_inventory, 0 },
    { "file-inventory", get_project_file_inventory, 0 },
    { "files", get_project_files, 0 },
    { "menu", get_project_menu, 0 },
    { "progress", get_project_progress, 0 },
    { "staff", get_project_staff, 0 },
    { "statistics", get_project_statistics, 0 },
    { "rss", get_rss, 0 },
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
