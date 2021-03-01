//
//      aegis - project change supervisor
//      Copyright (C) 2003-2008, 2011, 2012 Peter Miller
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
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>

#include <common/str_list.h>
#include <libaegis/change.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/emit/project.h>
#include <libaegis/http.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/rss.h>

#include <aeget/get/project/menu.h>


void
get_project_menu(project *pp, string_ty *, string_list_ty *)
{
    html_header(pp, 0);

    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf("</title></head><body>\n");
    html_header_ps(pp, 0);
    printf("<h1 align=center>\n");
    emit_project_but1(pp);
    printf("\n</h1>\n");

    change::pointer cp = pp->change_get();
    cstate_ty *cstate_data = cp->cstate_get();

    if (cstate_data->brief_description)
    {
        printf("<div class=\"brief-description\">\n");
        printf("<h2>Brief Description</h2>\n");
        html_encode_string(cstate_data->brief_description);
        printf("\n</div>\n");
    }

    if (cstate_data->description)
    {
        printf("<div class=\"description\">\n");
        printf("<h2>Description</h2>\n");
        html_encode_string(cstate_data->description);
        printf("\n</div>\n");
    }

    printf("<div class=\"information\">\n");
    printf("<h2>Information Available</h2>\n");
    printf("<dl>\n");

    //
    // Links to sub branches.
    //
    if
    (
        cstate_data->branch
    &&
        cstate_data->branch->sub_branch
    &&
        cstate_data->branch->sub_branch->length
    )
    {
        printf("<dt>Branches<dd>");
        cstate_branch_sub_branch_list_ty *bp = cstate_data->branch->sub_branch;
        for (size_t j = 0; j < bp->length; ++j)
        {
            if (j)
                printf(",\n");
            printf("<a href=\"%s/", http_script_name());
            html_escape_string(project_name_get(pp));
            long n = magic_zero_decode(bp->list[j]);
            printf(".%ld/?menu\">", n);
            html_encode_string(project_name_get(pp));
            printf(".%ld</a>", n);
        }
        printf("\n<p>\n");
    }

    printf("<dt>");
    emit_project_href(pp);
    printf("Baseline</a><dd>\n");
    printf("This item will provide you with access to the files in\n");
    printf("the project baseline.\n");
    if (!pp->is_a_trunk())
    {
        printf("The immediate baseline and all ancestor baselines are\n");
        printf("unioned and presented as a single directory tree.\n");
    }
    printf("<p>\n");
    printf("If you wish to download the sources using\n");
    printf("&ldquo;wget -r&rdquo; or similar, use the\n<i>");
    emit_project_href(pp, "file+contents+noindex+nolinks+noderived");
    printf("no navigation links</a></i> variant.\n");
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "aedist");
    printf("Download</a><dd>\n");
    printf("This item allows you to download the project in Aegis' own\n");
    printf("transfer format.  It preserves much of the meta-data for\n");
    printf("the project.  You unpack this format using the <i>aedist\n");
    printf("-receive</i> command.\n");
    if (!pp->is_a_trunk())
    {
        printf("You can also download the branch as a change set, see the ");
        emit_change_href(pp->change_get(), "menu");
        printf("corresponding change</a>'s ");
        emit_change_href(pp->change_get(), "download");
        printf("download</a> page.\n");
    }
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "changes");
    printf("Changes</a><dd>\n");
    printf("This item will provide you with a list of changes to this\n");
    printf("project, including those which have not yet commenced, those\n");
    printf("in progress, and those which have been completed.\n");

    printf("<p>\n");
    printf("Change lists are also available broken down by change state:\n");

    printf("<table>\n");
    printf("<tr><td><ul><li>\n");
    emit_project_href(pp, "changes+awaiting_development");
    printf("awaiting development</a>");
    printf("</ul></td><td>\n");
    nstring rss_filename = rss_feed_filename(pp, "awaiting_development");
    if (!rss_filename.empty())
    {
        emit_rss_icon_with_link(pp, os_basename(rss_filename));
    }
    printf("</td></tr>\n");

    printf("<tr><td><ul><li>\n");
    emit_project_href(pp, "changes+being_developed");
    printf("being developed</a>");
    printf("</ul></td><td>\n");
    rss_filename = rss_feed_filename(pp, "being_developed");
    if (!rss_filename.empty())
    {
        emit_rss_icon_with_link(pp, os_basename(rss_filename));
    }
    printf("</td></tr>\n");

    printf("<tr><td><ul><li>\n");
    emit_project_href(pp, "changes+awaiting_review");
    printf("awaiting review</a>");
    printf("</ul></td><td>\n");
    rss_filename = rss_feed_filename(pp, "awaiting_review");
    if (!rss_filename.empty())
    {
        emit_rss_icon_with_link(pp, os_basename(rss_filename));
    }
    printf("</td></tr>\n\n");

    printf("<tr><td><ul><li>\n");
    emit_project_href(pp, "changes+being_reviewed");
    printf("being reviewed</a>");
    printf("</ul></td><td>\n");
    rss_filename = rss_feed_filename(pp, "being_reviewed");
    if (!rss_filename.empty())
    {
        emit_rss_icon_with_link(pp, os_basename(rss_filename));
    }
    printf("</td></tr>\n");

    printf("<tr><td><ul><li>\n");
    emit_project_href(pp, "changes+awaiting_integration");
    printf("awaiting integration</a>");
    printf("</ul></td><td>\n");
    rss_filename = rss_feed_filename(pp, "awaiting_integration");
    if (!rss_filename.empty())
    {
        emit_rss_icon_with_link(pp, os_basename(rss_filename));
    }
    printf("</td></tr>\n");

    printf("<tr><td><ul><li>\n");
    emit_project_href(pp, "changes+being_integrated");
    printf("being integrated</a>");
    printf("</ul></td><td>\n");
    rss_filename = rss_feed_filename(pp, "being_integrated");
    if (!rss_filename.empty())
    {
        emit_rss_icon_with_link(pp, os_basename(rss_filename));
    }
    printf("</td></tr>\n\n");

    printf("<tr><td><ul><li>\n");
    emit_project_href(pp, "changes+completed");
    printf("completed</a>");
    printf("</ul></td><td>\n");
    rss_filename = rss_feed_filename(pp, "completed");
    if (!rss_filename.empty())
    {
        emit_rss_icon_with_link(pp, os_basename(rss_filename));
    }
    printf("</td></tr>\n\n");

    printf("<tr><td><ul><li>\n");
    emit_project_href(pp, "changes+not+completed");
    printf("not completed</a>\n");
    printf("</ul></td><td>\n");
    printf("</td></tr>\n");

    printf("</table>\n");
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "inventory");
    printf("Change Set Inventory</a><dd>\n");
    printf("This item allows you to obtain a list of all available\n");
    printf("change sets and their corresponding UUIDs.  This may be\n");
    printf("used to automate downloading of change sets not yet in\n");
    printf("your repository.<p>\n");

    printf("<dt>");
    emit_project_href(pp, "staff");
    printf("Staff</a><dd>\n");
    printf("This item will provide you with a list of staff\n");
    printf("involved in the project, both past and present. The\n");
    printf("list includes some statistics about the activities\n");
    printf("performed by the staff.\n");
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "statistics");
    printf("Statistics</a><dd>\n");
    printf("This item will provide you with some statistics about\n");
    printf("changes within the project.  The statistics include\n");
    printf("change causes, file activity and file types.\n");
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "files");
    printf("Files</a><dd>\n");
    printf("This item will provide you with a listing of the files\n");
    printf("which are the project master source.  Links are provided to\n");
    printf("individual file information.\n");
    if (!pp->is_a_trunk())
    {
        printf("To view only those files changed by this branch, see the ");
        emit_change_href(pp->change_get(), "menu");
        printf("corresponding change</a>'s ");
        emit_change_href(pp->change_get(), "files");
        printf("file list</a>.\n");
    }
    printf("<p>\n");

    printf("<dt>\n");
    emit_project_href(pp, "file+metrics");
    printf("File Metrics</a><dd>\n");
    printf("This item will provide you with a listing of the files\n");
    printf("with file metrics.  There are no default metrics; each\n");
    printf("project defines its own metrics.  Links are provided\n");
    printf("to individual file information.<p>\n");

    printf("<dt>\n");
    emit_project_href(pp, "file+activity");
    printf("File Activity</a><dd>\n");
    printf("This item will provide you with a listing of files\n");
    printf("which are actively being modified.  The list includes\n");
    printf("who is working on the changes, and a brief description\n");
    printf("of each change.  Where a file is common to more than\n");
    printf("one change, all changes are listed against the file.\n");
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "file+conflict");
    printf("File Conflict</a><dd>\n");
    printf("This item will provide you with a listing of changes which\n");
    printf("are actively modifying files <strong>if</strong> more than\n");
    printf("<em>one</em> change is modifying the file at the same time.\n");
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "file+history");
    printf("File History</a><dd>\n");
    printf("This item will provide you with a listing of files and\n");
    printf("the order in which all completed changes affected them.\n");
    printf("The ");
    emit_project_href(pp, "history+detailed");
    printf("detailed version</a> can take a long time to generate.\n");
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "project+history");
    printf("History</a><dd>\n");
    printf("This item will provide you with a listing of completed\n");
    printf("changes in the order in which they were completed.\n");
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "project+integration-histogram");
    printf("Integration Histogram</a><dd>\n");
    printf("This item will provide you with a histogram of changes\n");
    printf("completed over time.\n");
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "project+progress");
    printf("Progress Histogram</a><dd>\n");
    printf("This item will provide you with a histogram of change\n");
    printf("state transitions over time.  This shows you work\n");
    printf("progressing through the process and contributing\n");
    printf("towards project progress.\n");
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "project+change-cause");
    printf("Change Cause Histogram</a><dd>\n");
    printf("This item will provide you with a histogram of change\n");
    printf("causes over time.  Only completed changes are shown.\n");
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "file+cause-density");
    printf("Change Cause by File</a><dd>\n");
    printf("This item will provide you with a histogram of change\n");
    printf("causes against the files changed.  Only completed\n");
    printf("changes are shown.\n");
    printf("</dl>\n");
    printf("</div>\n");

    printf("<hr>\n");
    printf("<p align=center class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a>\n", http_script_name());
    printf("]</p>\n");

    html_footer(pp, 0);
}


// vim: set ts=8 sw=4 et :
