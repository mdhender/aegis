//
// aegis - project change supervisor
// Copyright (C) 2003-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <common/sizeof.h>
#include <common/str_list.h>
#include <libaegis/change.h>
#include <libaegis/http.h>
#include <libaegis/project.h>

#include <aeget/get/change.h>
#include <aeget/get/change/file_invento.h>
#include <aeget/get/file.h>
#include <aeget/get/file/activity.h>
#include <aeget/get/file/cause_densit.h>
#include <aeget/get/file/contents.h>
#include <aeget/get/file/diff.h>
#include <aeget/get/file/history.h>
#include <aeget/get/file/menu.h>
#include <aeget/get/file/metrics.h>
#include <aeget/get/project.h>


static void
project_handoff(change::pointer cp, string_ty *filename,
    string_list_ty *modifier)
{
    get_project(cp->pp, filename, modifier);
}


struct table_ty
{
    const char *name;
    void (*action)(change::pointer , string_ty *, string_list_ty *);
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
    { "change", get_change, 1 },
    { "file", get_file, 1 },

    { "activity", get_file_activity, 0 },
    { "cause-density", get_file_cause_density, 0 },
    { "conflict", get_file_activity, 0 },
    { "contents", get_file_contents, 0 },
    { "diff", get_file_diff, 0 },
    { "history", get_file_history, 0 },
    { "inventory", get_change_file_inventory, 0 },
    { "menu", get_file_menu, 0 },
    { "metrics", get_file_metrics, 0 },
};


static void
whine(change::pointer cp, string_list_ty *modifier)
{
    html_header(0, cp);
    printf("<title>Project ");
    html_encode_string(project_name_get(cp->pp));
    if (!cp->bogus)
        printf(", Change %ld", magic_zero_decode(cp->number));
    printf(",<br>\nPuzzlement</title></head><body>\n");
    html_header_ps(0, cp);
    printf("<h1 align=center>");
    emit_change(cp);
    printf(",<br>\nPuzzlement</h1>\n");
    printf("File information selector ");
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
        string_ty *s = str_format("file+%s", tp->name);
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
get_file(change::pointer cp, string_ty *filename, string_list_ty *modifier)
{
    //
    // Look for the project modifier to report.
    //
    if (modifier->nstrings >= 1)
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
    get_file_contents(cp, filename, modifier);
}


// vim: set ts=8 sw=4 et :
