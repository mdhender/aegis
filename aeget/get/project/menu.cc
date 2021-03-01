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
// MANIFEST: functions to manipulate menus
//

#include <ac/stdio.h>

#include <change.h>
#include <cstate.h>
#include <emit/project.h>
#include <get/project/menu.h>
#include <http.h>
#include <project.h>
#include <str_list.h>


void
get_project_menu(project_ty *pp, string_ty *filename, string_list_ty *modifier)
{
    change_ty       *cp;
    cstate_ty       *cstate_data;

    html_header(pp);

    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf("</title></head>\n<body><h1 align=center>\n");
    emit_project_but1(pp);
    printf("\n</h1>\n");

    cp = project_change_get(pp);
    cstate_data = change_cstate_get(cp);

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
	size_t          j;

	printf("<dt>Branches<dd>");
	for (j = 0; j < cstate_data->branch->sub_branch->length; ++j)
	{
	    if (j)
		printf(",\n");
	    printf("<a href=\"%s/", http_script_name());
	    html_escape_string(project_name_get(pp));
	    printf(".%ld/@@menu\">", cstate_data->branch->sub_branch->list[j]);
	    html_encode_string(project_name_get(pp));
	    printf(".%ld</a>", cstate_data->branch->sub_branch->list[j]);
	}
	printf("\n<p>\n");
    }

    printf("<dt><a href=\"%s/", http_script_name());
    html_escape_string(project_name_get(pp));
    printf("/\">Baseline</a><dd>\n");
    printf("This item will provide you with access to the files in\n");
    printf("the project baseline.\n");
    if (pp->parent)
    {
	printf("The immediate baseline and all ancestor baselines are\n");
	printf("unioned and presented as a single directory tree.\n");
    }
    printf("<p>\n");
    printf("If you wish to download the sources using\n");
    printf("&ldquo;wget -r&rdquo; or similar, use the\n<i>");
    emit_change_href(cp, "noindex@nolinks@noderived");
    printf("no navigation links</a></i> variant.\n");
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "aedist");
    printf("Download</a><dd>\n");
    printf("This item allows you to download the project in Aegis' own\n");
    printf("transfer format.  It preserves much of the meta-data for\n");
    printf("the project.  You unpack this format using the <i>aedist\n");
    printf("-receive</i> command.\n");
    if (pp->parent)
    {
	printf("You can also download the branch as a change set, see the ");
	emit_change_href(project_change_get(pp), "menu");
	printf("corresponding change</a>'s ");
	emit_change_href(project_change_get(pp), "download");
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
    printf("<ul>\n");
    printf("<li>");
    emit_project_href(pp, "changes@awaiting_development");
    printf("awaiting development</a>\n");
    printf("<li>");
    emit_project_href(pp, "changes@being_developed");
    printf("being developed</a>\n");
    printf("<li>");
    emit_project_href(pp, "changes@awaiting_review");
    printf("awaiting review</a>\n");
    printf("<li>");
    emit_project_href(pp, "changes@being_reviewed");
    printf("being reviewed</a>\n");
    printf("<li>");
    emit_project_href(pp, "changes@awaiting_integration");
    printf("awaiting integration</a>\n");
    printf("<li>");
    emit_project_href(pp, "changes@being_integrated");
    printf("being integrated</a>\n");
    printf("<li>");
    emit_project_href(pp, "changes@completed");
    printf("completed</a>\n");
    printf("<li>");
    emit_project_href(pp, "changes@not@completed");
    printf("not completed</a>\n");
    printf("</ul>\n");
    printf("<p>\n");

#if 0
    href = http_script_name() ## "?file@proj_staff+project@" ## quote_url(pn);
    printf("<dt><a href=\"" ## href ## "\">");
    printf("Staff");
    printf("</a><dd>");
    printf("This item will provide you with a list of staff involved in");
    printf("the project, both past and present.  The list includes some");
    printf("statistics about the activities performed by the staff.");
    printf("<p>\n");
#endif

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
    if (pp->parent)
    {
	printf("To view only those files changed by this branch, see the ");
	emit_change_href(project_change_get(pp), "menu");
	printf("corresponding change</a>'s ");
	emit_change_href(project_change_get(pp), "files");
	printf("file list</a>.\n");
    }
    printf("<p>\n");

    printf("<dt>\n");
    emit_project_href(pp, "file@metrics");
    printf("File Metrics</a><dd>");
    printf("This item will provide you with a listing of the files with\n");
    printf("file metrics.  (Each project defines its own metrics, none\n");
    printf("by default.)");
    printf("Links are provided to individual file information.");
    printf("<p>\n");

    printf("<dt>\n");
    emit_project_href(pp, "file@activity");
    printf("File Activity</a><dd>");
    printf("This item will provide you with a listing of files which are");
    printf("actively being modified.  The list includes who is working on");
    printf("the changes, and a brief description of each change.  Where a");
    printf("file is common to more than one change, all changes are listed");
    printf("against the file.");
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "file@conflict");
    printf("File Conflict</a><dd>");
    printf("This item will provide you with a listing of changes which\n");
    printf("are actively modifying files <strong>if</strong> more than\n");
    printf("<em>one</em> change is modifying the file at the same time.");
    printf("<p>\n");

    printf("<dt>");
    emit_project_href(pp, "file@history");
    printf("File History</a><dd>");
    printf("This item will provide you with a listing of files and the order");
    printf("in which all completed changes affected them.  The\n");
    emit_project_href(pp, "history@detailed");
    printf("detailed version</a> can take a long time to generate.");
    printf("<p>\n");

#if 0
    printf("<dt>");
    href = http_script_name() ## "?file@proj_hstry+project@" ## quote_url(pn);
    printf("<dt><a href=\"" ## href ## "\">");
    printf("History");
    printf("</a><dd>");
    printf("This item will provide you with a listing of completed changes");
    printf("in the order in which they were completed.");
    printf("<p>\n");

    printf("<dt>");
    href = http_script_name() ## "?file@proj_hstgm+project@" ## quote_url(pn);
    printf("<dt><a href=\"" ## href ## "\">");
    printf("Integration Histogram");
    printf("</a><dd>");
    printf("This item will provide you with a histogram of changes completed");
    printf("over time.");
    printf("<p>\n");

    printf("<dt>");
    href = http_script_name() ## "?file@proj_prgr1+project@" ## quote_url(pn);
    printf("<dt><a href=\"" ## href ## "\">");
    printf("Progress Histogram");
    printf("</a><dd>");
    printf("This item will provide you with a histogram of change state");
    printf("transitions over time.  This shows you work progressing through");
    printf("the process and contributing towards project progress.");
    printf("<p>\n");

    printf("<dt>");
    href = http_script_name() ## "?file@proj_cch1+project@" ## quote_url(pn);
    printf("<dt><a href=\"" ## href ## "\">");
    printf("Change Cause Histogram");
    printf("</a><dd>");
    printf("This item will provide you with a histogram of change causes");
    printf("over time.  Only completed changes are shown.");
    printf("<p>\n");

    printf("<dt>");
    href = http_script_name() ## "?file@file_densi+project@" ## quote_url(pn);
    printf("<dt><a href=\"" ## href ## "\">");
    printf("Change Cause by File");
    printf("</a><dd>");
    printf("This item will provide you with a histogram of change causes");
    printf("against the files changed.  Only completed changes are shown.");
#endif
    printf("</dl>\n");
    printf("</div>\n");

    printf("<hr>\n");
    printf("<p align=center class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a>\n", http_script_name());
    printf("]</p>\n");

    html_footer();
}
