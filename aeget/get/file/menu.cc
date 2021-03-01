//
// aegis - project change supervisor
// Copyright (C) 2003-2008, 2011, 2012 Peter Miller
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

#include <common/ac/assert.h>
#include <common/ac/stdio.h>

#include <common/debug.h>
#include <common/nstring.h>
#include <common/str_list.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/fstate.fmtgen.h>
#include <libaegis/http.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>

#include <aeget/get/file/menu.h>


void
get_file_menu(change::pointer cp, string_ty *filename, string_list_ty *)
{
    //
    // Emit the page title.
    //
    html_header(0, cp);
    printf("<title>Project\n");
    html_encode_string(project_name_get(cp->pp));
    if (!cp->bogus)
        printf(", Change %ld\n", magic_zero_decode(cp->number));
    printf(", File ");
    html_encode_string(filename);
    printf("</title></head><body>\n");

    //
    // Emit the page heading.
    //
    html_header_ps(0, cp);
    printf("<h1 align=center>\n");
    emit_change(cp);
    printf(",<br>File &ldquo;");
    html_encode_string(filename);
    printf("&rdquo;</h1>\n");

    //
    // Make sure the file exists.
    //
    fstate_src_ty *src = 0;
    if (!cp->bogus)
    {
        src = cp->file_find(nstring(filename), view_path_first);
        if (!src)
        {
            printf("This file is not part of this change.\n");
            if (cp->pp->file_find(filename, view_path_simple))
            {
                printf("You may like to see the ");
                cp->bogus = 1;
                emit_file_href(cp, filename, "menu");
                cp->bogus = 0;
                printf("project file menu</a> instead.\n");
            }
            else
            {
                printf("It may be created by the build; ");
                emit_file_href(cp, filename, 0);
                printf("if it exists</a> it is unmanaged.");
            }

            //
            // Emit the page footer.
            //
            html_footer(0, cp);
            return;
        }
    }
    else
    {
        src = cp->pp->file_find(filename, view_path_simple);
        if (!src)
        {
            printf("This file is not controlled by Aegis.\n");
            printf("It may have been created by the build;\n");
            emit_file_href(cp, filename, 0);
            printf("if it exists</a> it is unmanaged.");

            //
            // Emit the page footer.
            //
            html_footer(0, cp);
            return;
        }
    }
    assert(src);

    printf("<div class=\"information\">\n");
    printf("<dl>\n");
    if (src->uuid)
    {
        printf("<td>UUID<dd><tt>");
        html_escape_string(src->uuid);
        printf("</tt><p>\n");
    }

    if (!cp->bogus)
    {
        printf("<dt>");
        cp->bogus = 1;
        emit_file_href(cp, filename, "menu");
        cp->bogus = 0;
        printf("Project File</a><dd>\n");
        printf("This item directs you to the corresponding file menu\n");
        printf("for the project.\n");
        printf("<p>\n");
    }
    if (src->attribute && src->attribute->length)
    {
        printf("<dt>Attributes<dd>\n");
        printf("<table>\n");
        printf("<tr><th>Name</th><th>Value</th></tr>\n");
        attributes_list_ty *alp = src->attribute;
        for (size_t j = 0; j < alp->length; ++j)
        {
            attributes_ty *ap = alp->list[j];
            printf("<tr><td>\n");
            if (ap->name)
                printf("%s\n", nstring(ap->name).html_quote().c_str());
            printf("</td><td>\n");
            if (ap->value)
                printf("%s\n", nstring(ap->value).html_quote().c_str());
            printf("</td></tr>\n");
        }
        printf("</table>\n");
        printf("<p>\n");
    }

    printf("<dt>");
    emit_file_href(cp, filename, "activity");
    printf("File Activity</a><dd>\n");
    printf("This item will provide you with a listing of changes which\n");
    printf("are actively modifying this file.  The list includes who\n");
    printf("is working on the changes, and a brief description of each\n");
    printf("change.  Where a file is common to more than one change,\n");
    printf("all changes are listed against the file.\n");

    printf("<p><dt>");
    emit_file_href(cp, filename, "conflict");
    printf("File Conflict</a><dd>\n");
    printf("This item will provide you with a listing of changes which are\n");
    printf("actively modifying this file <strong>if</strong> more than\n");
    printf("<em>one</em> change is modifying the file.\n");

    printf("<p><dt>");
    int hold_bogus = cp->bogus;
    cp->bogus = 1;
    emit_file_href(cp, filename, "file+history");
    cp->bogus = hold_bogus;
    printf("File History</a><dd>\n");
    printf("This item will provide you with a listing of all completed\n");
    printf("changes which affected this file in the past.  The\n");
    emit_file_href(cp, filename, "file+history+detailed");
    printf("detailed version</a> will take longer to generate.\n");
    printf("<p>\n");
    printf("For historical versions of this file, click on the\n");
    printf("links in the <strong>Edit</strong> column.  Each links\n");
    printf("you to contents of the version of the file integrated\n");
    printf("for that change.\n");

    if (cp->bogus)
    {
        if (src->deleted_by)
        {
            printf("<p><dt>Baseline<dd>\n");
            printf("The file no longer exists in the baseline.\n");
            // FIXME: add link.
            printf("It was deleted by change %ld.\n", src->deleted_by);
        }
        else
        {
            printf("<p><dt>");
            emit_file_href(cp, filename, 0);
            printf("Baseline</a><dd>\n");
            printf("This item gives access to the file contents as it\n");
            printf("exists in the curremt project baseline.\n");
        }
    }
    else
    {
        switch (src->action)
        {
        case file_action_create:
            if (!cp->is_completed())
            {
                printf("<p><dt>Baseline<dd>\n");
                printf("The file does not yet exist in the baseline.\n");
                printf("It is being created by this change, which is not\n");
                printf("yet integrated\n");
                break;
            }
            // fall through...

        case file_action_modify:
        case file_action_remove:
        case file_action_insulate:
        case file_action_transparent:
#ifndef DEBUG
        default:
#endif
            printf("<p><dt>");
            cp->bogus = 1;
            emit_file_href(cp, filename, 0);
            cp->bogus = 0;
            printf("Baseline</a><dd>\n");
            printf("This item will show you the contents of the file in\n");
            printf("the project baseline, the <strong>current</strong>\n");
            printf("master version.\n");
            break;
        }
    }

    if (cp->bogus || cp->is_completed())
    {
        printf("<p><dt>");
        emit_file_href(cp, filename, "diff+detailed");
        printf("Differences</a><dd>\n");
        printf("This item will show you a grid of historical\n");
        printf("file versions, allowing you to obtain diff(1)\n");
        printf("patches between all of the versions shown.\n");
    }

    printf("</dl>\n");
    printf("</div>\n");

    //
    // Emit the navigation bar.
    //
    printf("<hr>\n");
    printf("<p align=center class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(cp->pp, "menu");
    printf("Project Menu</a> |\n");
    if (!cp->bogus)
    {
        emit_change_href(cp, "menu");
        printf("Change Menu</a> |\n");
    }
    emit_change_href(cp, "files");
    printf("File List</a>\n");
    printf("]</p>\n");

    //
    // Emit page footer.
    //
    html_footer(0, cp);
}


// vim: set ts=8 sw=4 et :
