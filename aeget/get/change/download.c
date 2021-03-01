/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate downloads
 */

#include <ac/stdio.h>

#include <change.h>
#include <cstate.h>
#include <get/change/download.h>
#include <http.h>
#include <project.h>
#include <str_list.h>


static int
not_awaiting_development(change_ty *cp)
{
    cstate          cstate_data;

    cstate_data = change_cstate_get(cp);
    return (cstate_data->state != cstate_state_awaiting_development);
}


void
get_change_download(change_ty *cp, string_ty *fn, string_list_ty *modifier)
{
    int             ok;

    html_header(cp->pp);
    printf("<title>Project\n");
    html_encode_string(project_name_get(cp->pp));
    if (!cp->bogus)
	printf(", Change %ld\n", cp->number);
    printf("</title></head>\n<body><h1 align=center>\n");
    emit_change(cp);
    printf(",<br>\nDownload</h1>\n");
    printf("<dl>\n");

    ok = not_awaiting_development(cp);

    printf("<dt>");
    emit_change_href(cp, "aedist");
    printf("aedist</a>");
    if (ok)
    {
	printf(" (");
	emit_change_href(cp, "aedist@es");
	printf("entire source</a>)");
    }
    printf("<dd>\n");
    printf("This item allows you to download the change set in Aegis'\n");
    printf("own transfer format.  It preserves most meta-data for the\n");
    printf("change set.  You unpack this format using the <i>aedist\n");
    printf("-receive</i> command.\n");
    printf("<p>\n");

    if (ok)
    {
	printf("<dt>");
	emit_change_href(cp, "aepatch");
	printf("patch</a><dd>");
	printf("This item allows you to download change sets as a patch.\n");
	printf("It loses most of the meta-data for the change set (there\n");
	printf("is nowhere in the diff format to put it).  You unpack\n");
	printf("this format using one of the <i>zcat | patch -p0</i>\n");
	printf("or <i>aepatch -receive</i> commands.\n");
	printf("<p>\n");

	printf("<dt>");
	emit_change_href(cp, "aetar");
	printf("tar.gz</a> (");
	emit_change_href(cp, "aetar@es");
	printf("entire source</a>)<dd>\n");
	printf("This item allows you to download change sets as a tarball.\n");
	printf("It loses most of the meta-data for the change set (there\n");
	printf("is nowhere in the tar format to put it).  You unpack\n");
	printf("this format using one of the <i>tar xzf</i> or <i>aetar\n");
	printf("-receive</i> commands.\n");
	printf("<p>\n");
    }

    printf("<dt> Pre-4.10 compatibility ");
    emit_change_href(cp, "aedist@nopatch");
    printf("aedist</a>");
    if (ok)
    {
	printf(" (");
	emit_change_href(cp, "aedist@nopatch@es");
	printf("entire source</a>)");
    }
    printf("<dd>\n");
    printf("This item allows you to download change sets in Aegis' own\n");
    printf("transfer format, but omitting items which older versions\n");
    printf("of <i>aedist</i> do not understand.  It preserves most\n");
    printf("meta-data for the change set.  You unpack this format using\n");
    printf("the <i>aedist -receive</i> command.\n");

    printf("</dl>\n");
    printf("<p>\n");
    printf("The <em>entire source</em> options include the rest of\n");
    printf("the project source files in the downloaded change set.\n");
    html_footer();
}
