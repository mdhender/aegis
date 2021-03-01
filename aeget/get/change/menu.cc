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
#include <get/change/menu.h>
#include <http.h>
#include <project.h>
#include <str_list.h>


void
get_change_menu(change_ty *cp, string_ty *filename, string_list_ty *modifier)
{
    cstate_ty       *cstate_data;

    html_header(cp->pp);

    printf("<title>Project\n");
    html_encode_string(project_name_get(cp->pp));
    printf(", Change %ld\n", magic_zero_decode(cp->number));
    printf("</title></head>\n<body><h1 align=center>\n");
    emit_change_but1(cp);
    printf("</h1>\n");

    cstate_data = change_cstate_get(cp);
    if (cstate_data->brief_description)
    {
	printf("<div class=\"brief-description\">\n");
	printf("<h2>Brief Description</h2>\n");
	html_encode_string(cstate_data->brief_description);
	printf("</div>\n");
    }

    if (cstate_data->description)
    {
	printf("<div class=\"description\">\n");
	printf("<h2>Description</h2>\n");
	html_encode_string(cstate_data->description);
	printf("</div>\n");
    }

    printf("<div class=\"information\">\n");
    printf("<h2>Information Available</h2>\n");
    printf("<dl>\n");

    printf("<dt>State<dd>This change is in the ");
    printf("<dfn>%s</dfn> state.\n", cstate_state_ename(cstate_data->state));
    printf("<p>\n");

    if (cstate_data->branch)
    {
	printf("<dt><a href=\"%s/", http_script_name());
	html_escape_string(project_name_get(cp->pp));
	printf(".%ld/@@menu\">Branch</a><dd>\n", magic_zero_decode(cp->number));
	printf("This change is a branch of the &ldquo;");
	html_encode_string(project_name_get(cp->pp));
	printf("&rdquo; project.\n");
	printf("This item takes you to a project menu for the\n&ldquo;");
	html_encode_string(project_name_get(cp->pp));
	printf(".%ld&rdquo; branch.\n", magic_zero_decode(cp->number));
	printf("<p>\n");
    }

    printf("<dt>");
    emit_change_href(cp, "download");
    printf("Download</a><dd>\n");
    printf("This item will provide you with links to several different\n");
    printf("download formats.  Select the one appropriate to you needs.\n");
    printf("<p>\n");

    if (cstate_data->state >= cstate_state_being_developed)
    {
	printf("<dt>");
	emit_change_href(cp, "files");
	printf("Files</a><dd>\n");
	printf("This item will provide you with a listing of files which\n");
	printf("are being created, modified or deleted by this change.\n");
	printf("<p>\n");
    }

    if
    (
	cstate_data->state >= cstate_state_being_developed
    &&
	cstate_data->state < cstate_state_being_integrated
    )
    {
	printf("<dt>");
	emit_change_href(cp, 0);
	printf("Development Directory</a><dd>\n");
	printf("This item will provide you with access to the files in\n");
	printf("the development directory.\n");
	printf("This will be unioned with all baselines\n");
	printf("and presented as a single directory tree.\n");
	printf("<p>\n");
        printf("If you wish to download the sources using\n");
	printf("&ldquo;wget -r&rdquo; or similar, use the\n<i>");
	emit_change_href(cp, "noindex@nolinks@noderived");
        printf("no navigation links</a></i> variant.\n");
	printf("<p>\n");
    }

    if (cstate_data->state == cstate_state_being_integrated)
    {
	printf("<dt>");
	emit_change_href(cp, 0);
	printf("Integration Directory</a><dd>\n");
	printf("This item will provide you with access to the files in\n");
	printf("the development directory.\n");
	printf("This will be unioned with all ancestor baselines\n");
	printf("and presented as a single directory tree.\n");
	printf("<p>\n");
    }

    if
    (
	cstate_data->state >= cstate_state_being_developed
    &&
	cstate_data->state <= cstate_state_being_integrated
    )
    {
	printf("<dt>\n");
	emit_change_href(cp, "activity");
	printf("File Activity</a><dd>\n");
        printf("This item will provide you with a listing of files which\n");
        printf("are being modified in this change <em>and also</em>\n");
        printf("in other changes.  The list includes who is working on\n");
        printf("the changes, and a brief description of each change.\n");
	printf("<p>\n");

	printf("<dt>\n");
	emit_change_href(cp, "conflict");
	printf("File Conflict</a><dd>\n");
        printf("This item will provide you with a list of changes which\n");
        printf("are actively modifying files in common with this change\n");
        printf("<strong>if</strong> more than <em>one</em> change is\n");
        printf("modifying the file at the same time.\n");
	printf("<p>\n");
    }

    if (cstate_data->state >= cstate_state_being_developed)
    {
	printf("<dt>\n");
	emit_change_href(cp, "file@history");
	printf("File History</a><dd>\n");
	printf("This item will provide you with a listing of all completed\n");
	printf("changes which affected the files in this change.  The\n");
	emit_change_href(cp, "file@history@deatiled");
	printf("detailed version</a> can take a long time to generate.\n");
	printf("<p>\n");
    }

#if 0
    printf("<dt>\n");
    href = script_name ## "?file@chan_hstry+project@" ## quote_url(pn) ##
	"+change@" ## cn;
    printf("<a href=\"" ## href ## "\">\n");
    printf("History\n");
    printf("</a><dd>\n");
    printf("This item will provide you with a listing of the state\n");
    printf("transitions of this change, when they were performed, and\n");
    printf("who performed them.\n");
    printf("<p>\n");

    if (c.state >= being_developed)
    {
	printf("<dt>Download<dd>\n");
	printf("There are three ways to download this change.\n");
	printf("<br><table align=center>\n");

	printf("<tr><td valign=top>\n");
	href = script_name ## "/" ## quote_url(pn) ## ".C" ## cn
	    ## ".patch?file@aepatch+project@" ## quote_url(pn)
	    ## "+change@" ## cn;
	printf("<a href=\"" ## href ## "\"><code>patch</code></a>\n");
	printf("</td><td valign=top>\n");
	printf("This link will download a compressed patch familiar to Open\n");
	printf("Source developers.\n");
	printf("Unlike the <i>aedist</i> link, below, the change meta-data\n");
	printf("is not preserved, only the change description endures.\n");
	printf("</td></tr>\n");

	printf("<tr><td valign=top>\n");
	href = script_name ## "/" ## quote_url(pn) ## ".C" ## cn
	    ## ".tar.gz?file@aetar+project@" ## quote_url(pn)
	    ## "+change@" ## cn;
	printf("<a href=\"" ## href ## "\"><code>tar.gz</code></a>\n");
	printf("</td><td valign=top>\n");
	printf("This link will download a complete project source tarball\n");
	printf("up to this change.  <strong>Note:</strong> These tarballs\n");
	printf("do not contain derived or generated files.  Unlike the\n");
	printf("<i>aedist</i> link, below, the change meta-data is not\n");
	printf("preserved.\n");
	printf("</td></tr>\n");

	printf("<tr><td valign=top>\n");
	href = script_name ## "/" ## quote_url(pn) ## ".C" ## cn
	    ## ".ae?file@aedist+project@" ## quote_url(pn)
	    ## "+change@" ## cn;
	printf("<a href=\"" ## href ## "\"><code>aedist</code></a>\n");
	printf("</td><td valign=top>\n");
	printf("The <em>aedist</em>(1) program packages changes so\n");
	printf("that they may be reproduced exactly on the remote\n");
	printf("site.  Changes packaged in this way require the\n");
	printf("<em>aedist</em>(1) program from Aegis 4.23 or later to\n");
	printf("apply them.\n");
	printf("</td></tr>\n");

	printf("</table>\n");
    }
#endif

    printf("</dl></div>\n");

    printf("<hr>\n");
    printf("<p align=center class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(cp->pp, "menu");
    printf("Project Menu</a> |\n");
    emit_project_href(cp->pp, "changes");
    printf("Change List</a>\n");
    printf("]</p>\n");

    html_footer();
}
