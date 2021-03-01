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
// MANIFEST: functions to manipulate downloads
//

#include <ac/stdio.h>

#include <change.h>
#include <cstate.h>
#include <get/change/download.h>
#include <http.h>
#include <project.h>
#include <str_list.h>
#include <version_stmp.h>


static int
not_awaiting_development(change_ty *cp)
{
    cstate_ty       *cstate_data;

    cstate_data = change_cstate_get(cp);
    return (cstate_data->state != cstate_state_awaiting_development);
}


void
get_change_download(change_ty *cp, string_ty *fn, string_list_ty *modifier)
{
    int             ok;

    html_header(0, cp);
    printf("<title>Project\n");
    html_encode_string(project_name_get(cp->pp));
    if (!cp->bogus)
	printf(", Change %ld\n", magic_zero_decode(cp->number));
    printf("</title></head><body>\n");
    html_header_ps(0, cp);
    printf("<h1 align=center>\n");
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
	emit_change_href(cp, "aedist+es");
	printf("entire source</a>)");
    }
    printf("<dd>\n");
    printf("This item allows you to download the change set\n");
    printf("in Aegis' own transfer format.  It preserves most\n");
    printf("meta-data for the change set.  You unpack this format\n");
    printf("using the &ldquo;<i>aedist -receive</i>&rdquo; command.\n");
    printf("<p>\n");
    printf("For the terminally curious, the term &ldquo;Aegis'\n");
    printf("own transfer format&rdquo; means an optionally BASE64\n");
    printf("encoded, optionally gzipped, CPIO archive.  The\n");
    printf("top-level directory indicates whether the archive file\n");
    printf("is a source file, meta-data, <em>etc</em>.\n");
    printf("<p>\n");

    if (ok)
    {
	printf("<dt>");
	emit_change_href(cp, "aepatch");
	printf("patch</a> (");
	emit_change_href(cp, "aepatch+compat=4.16");
        printf("no meta data</a>)<dd>This item allows you to\n");
        printf("download a change set as a conventional patch.\n");
        printf("It even preserves most of the meta-data for the\n");
        printf("change set.  You unpack this format using one\n");
        printf("of the &ldquo;<i>zcat | patch -p0</i>&rdquo; or\n");
        printf("&ldquo;<i>aepatch -receive</i>&rdquo; commands.\n");
	printf("<p>\n");

	printf("<dt>");
	emit_change_href(cp, "aetar");
	printf("tar.gz</a> (");
	emit_change_href(cp, "aetar+es");
        printf("entire source</a>)<dd>This item allows you\n");
        printf("to download change sets as a tarball.  It\n");
        printf("loses most of the meta-data for the change set\n");
        printf("(there is nowhere in the tar format to put\n");
        printf("it).  You unpack this format using one of the\n");
        printf("&ldquo;<i>tar xzf</i>&rdquo; or &ldquo;<i>aetar\n");
        printf("-receive</i>&rdquo; commands.\n");
        printf("<p>All the files in the tarball have the project\n");
        printf("name and version added as a default path prefix, so\n");
        printf("they unpack into a single directory.  If you don't\n");
        printf("want the extra top-level directory, use these links:\n");
	emit_change_href(cp, "aetar+noprefix");
	printf("tar.gz</a> (");
	emit_change_href(cp, "aetar+noprefix+es");
	printf("entire source</a>).<p>\n");
    }

    printf("<dt> ");
    emit_change_href(cp, "aedist+compat=4.6");
    printf("Pre-4.7 aedist</a>");
    if (ok)
    {
	printf(" (");
	emit_change_href(cp, "aedist+compat=4.6+es");
	printf("entire source</a>)");
    }
    printf(", ");
    emit_change_href(cp, "aedist+compat=4.9");
    printf("pre-4.10 aedist</a>");
    if (ok)
    {
	printf(" (");
	emit_change_href(cp, "aedist+compat=4.9+es");
	printf("entire source</a>)");
    }
    printf(", ");
    emit_change_href(cp, "aedist+compat=4.16");
    printf("pre-4.17 aedist</a>");
    if (ok)
    {
	printf(" (");
	emit_change_href(cp, "aedist+compat=4.16+es");
	printf("entire source</a>)");
    }
    printf(" compatibility<dd>\n");
    printf("These items allow you to download change sets in\n");
    printf("Aegis' own transfer format, but omitting items which\n");
    printf("older versions of <i>aedist</i> do not understand.\n");
    printf("It preserves most meta-data for the change set.\n");
    printf("You unpack this format using the &ldquo;<i>aedist\n");
    printf("-receive</i>&rdquo; command.\n");
    printf("<p>\n");
    printf("The version of <i>aedist</i> used by this server is %s.\n",
	version_stamp());
    printf("To work out what version of <i>aedist</i> you\n");
    printf("have at your end, use the &ldquo;<tt>aedist\n");
    printf("--version</tt>&rdquo; command.\n");

    printf("</dl>\n");
    printf("<p>\n");
    printf("A downloaded change set usually contains only the files\n");
    printf("in the changset itself.  The <em>entire source</em>\n");
    printf("options, above, include the rest of the project source\n");
    printf("files in the downloaded change set.\n");
    html_footer(0, cp);
}
