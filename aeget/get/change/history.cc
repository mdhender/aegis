//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: implementation of the get_change_history class
//

#include <ac/stdio.h>

#include <aer/func/now.h> // for working_days
#include <change.h>
#include <cstate.h>
#include <error.h> // for assert
#include <get/change/history.h>
#include <http.h>
#include <now.h>
#include <project.h>
#include <symtab.h>
#include <user.h>


static void
user_reaper(void *p)
{
    user_free((user_ty *)p);
}


void
get_change_history(change_ty *cp, string_ty *filename, string_list_ty *)
{
    //
    // Emit page title.
    //
    html_header(0, cp);
    printf("<title>Project\n");
    html_encode_string(project_name_get(cp->pp));
    if (!cp->bogus)
	printf(", Change %ld", magic_zero_decode(cp->number));
    printf(", History\n");
    printf("</title></head><body>\n");

    //
    // Emit page header.
    //
    html_header_ps(0, cp);
    printf("<h1 align=center>\n");
    emit_change(cp);
    printf(",<br>\nHistory</h1>\n");

    //
    // Emit column headings.
    //
    printf("<div class=\"information\"><table align=center>\n");
    printf("<tr class=\"even-group\"><th>What</th><th>When</th><th>Who</th>");
    printf("<th>Comment</th></tr>\n");
    cstate_ty *cstate_data = change_cstate_get(cp);
    assert(cstate_data);
    assert(cstate_data->history);
    symtab_ty users;
    users.set_reap(user_reaper);
    int rownum = 0;
    for (size_t j = 0; j < cstate_data->history->length; ++j)
    {
	//
	// Emit the state transition.
	//
	cstate_history_ty *hp = cstate_data->history->list[j];
	const char *html_class = (((rownum++ / 3) & 1) ?  "even" : "odd");
	printf("<tr class=\"%s-group\">", html_class);
	printf("<td valign=top>%s</td>\n", cstate_history_what_ename(hp->what));

	//
	// Emit the time stamp.
	//
	printf("<td valign=top>%.24s</td>\n", ctime(&hp->when));

	//
	// Emit the user.
	//
	printf("<td valign=top>");
	printf("<a href=\"mailto:");
	user_ty *up = (user_ty *)users.query(hp->who);
	if (!up)
	{
	    up = user_symbolic(cp->pp, hp->who);
	    users.assign(hp->who, up);
	}
	html_escape_string(user_email_address(up));
	printf("\">");
	html_encode_string(user_name2(up));
	printf("</a></td>\n");

	//
	// Emit the comment.
	//
	printf("<td valign=top>");
	if (hp->why && hp->why->str_length)
	    html_encode_string(hp->why);

	time_t t2 =
	    (
		j + 1 >= cstate_data->history->length
	    ?
		now()
	    :
		cstate_data->history->list[j + 1]->when
	    );
	double wd = working_days(hp->when, t2);
	if (wd >= 0.1)
	{
	    if (hp->why && hp->why->str_length)
		printf("<br>\n");
	    printf("Elapsed: %5.3f working days.", wd);
	}
	printf("</td></tr>\n");
    }

    //
    // Emit sumamry row.
    //
    printf("<tr class=\"even-group\"><td colspan=4>\nListed ");
    printf("%ld state transitions.\n", (long)cstate_data->history->length);
    printf("</td></tr>\n");
    printf("</table></div>\n");

    //
    // Emit command line equivalent.
    //
    printf("<hr>\n");
    printf("<div class=\"report-cmd\">\n");
    printf("A similar report may be obtained from the command line,");
    printf("with one of<blockquote><pre>ael change_history -p ");
    html_encode_string(project_name_get(cp->pp));
    printf(" -c %ld\n", magic_zero_decode(cp->number));
    printf("aer change_history -p ");
    html_encode_string(project_name_get(cp->pp));
    printf(" -c %ld", magic_zero_decode(cp->number));
    printf("</pre></blockquote></div>\n");

    //
    // Emit navigation bar.
    //
    printf("<hr>\n");
    printf("<p align=center class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(cp->pp, "menu");
    printf("Project Menu</a> |\n");
    emit_project_href(cp->pp, "changes");
    printf("Change List</a>\n");
    if (!cp->bogus)
    {
	printf("| ");
	emit_change_href(cp, "menu");
	printf("Change Menu</a>\n");
    }
    printf("]</p>\n");

    //
    // Emit page footer.
    //
    html_footer(0, cp);
}
