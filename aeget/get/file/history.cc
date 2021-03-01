//
//	aegis - project change supervisor
//	Copyright (C) 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate historys
//

#include <ac/stdio.h>
#include <ac/string.h>

#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <cstate.h>
#include <emit/brief_descri.h>
#include <emit/edit_number.h>
#include <error.h> // for assert
#include <get/file/history.h>
#include <http.h>
#include <project.h>
#include <project/file/roll_forward.h>
#include <str_list.h>
#include <trace.h>


void
get_file_history(change_ty *master_cp, string_ty *a_filename,
    string_list_ty *modifier)
{
    trace(("get_file_history()\n{\n"));
    nstring filename(str_copy(a_filename));
    bool detailed = false;
    bool all = false;
    if (filename == ".")
	all = true;
    for (size_t k = 0; k < modifier->nstrings; ++k)
    {
	if (0 == strcasecmp(modifier->string[k]->str_text, "detailed"))
	{
	    detailed = true;
	    break;
	}
    }

    //
    // Reconstruct the project file history.
    //
    time_t when = change_completion_timestamp(master_cp);
    project_ty *pp = master_cp->pp;
    project_file_roll_forward historian(pp, when, detailed);
    nstring_list key;
    historian.keys(key);

    html_header(pp, master_cp);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(", File History</title></head><body>\n");
    html_header_ps(pp, master_cp);
    printf("<h1 align=center>");
    emit_change(master_cp);
    printf(",<br>\nFile History</h1>\n");

    printf("<div class=\"information\"><table align=center>\n");
    printf("<tr class=\"even-group\"><th colspan=3>File Name</th>\n");
    printf("<th>Delta</th><th>Date and Time</th><th>Edit</th>\n");
    printf("<th>Description</th><th>&nbsp;</th></tr>\n");

    //
    // list the change's files' histories
    //
    size_t num_files = 0;
    for (size_t j = 0; j < key.size(); ++j)
    {
	nstring the_file_name = key[j];
	if (!all && filename != the_file_name)
	    continue;
	if
	(
	    !master_cp->bogus
	&&
	    !change_file_find(master_cp, the_file_name, view_path_first)
	)
	    continue;
	++num_files;

	printf("<tr class=\"even-group\"><td colspan=8>\n");
	emit_file_href(master_cp, the_file_name, "menu");
	html_encode_string(the_file_name);
	printf("</td></tr>\n");
	size_t num = 0;

	int usage_track = -1;
	int action_track = -1;

	file_event_list_ty *felp = historian.get(the_file_name);
	if (felp)
	{
	    for (size_t k = 0; k < felp->length; ++k)
	    {
		file_event_ty *fep = felp->item + k;
		assert(fep->src);
		if (!fep->src)
		    continue;

		const char *html_class =
		    (((num / 3) & 1) ?  "even-group" : "odd-group");
		++num;
		printf("<tr class=\"%s\">", html_class);

		// filename column
		printf("<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>\n");

		// usage column
		printf("<td valign=\"top\">");
		if (usage_track != fep->src->usage)
		{
		    printf("%s", file_usage_ename(fep->src->usage));
		    usage_track = fep->src->usage;
		}
		printf("</td>\n");

		// action column
		printf("<td valign=\"top\">");
		if (action_track != fep->src->action)
		{
		    printf("%s", file_action_ename(fep->src->action));
		    action_track = fep->src->action;
		}
		printf("</td>\n");

		// delta column
		printf("<td valign=\"top\">");
		emit_change_href(fep->cp, "menu");
		string_ty *s = change_version_get(fep->cp);
		html_encode_string(s);
		str_free(s);
		printf("</a></td>\n");

		// date and time column
		printf("<td valign=\"top\">");
		html_encode_charstar(ctime(&fep->when));
		printf("</td>\n");

		// change column
		printf("<td valign=\"top\" align=\"right\">");
		emit_file_href(fep->cp, fep->src->file_name, 0);
		emit_edit_number(fep->cp, fep->src);
		printf("</a></td>\n");

		// description column
		printf("<td valign=\"top\">\n");
		emit_change_brief_description(fep->cp);
		printf("</td>\n");

		// download column
		printf("<td valign=\"top\">");
		emit_change_href(fep->cp, "download");
		printf("Download</a></td></tr>\n");
	    }
	}

	//
	// Now output details of this change, as the "end"
	// of the history.
	//
	if (!master_cp->bogus && !change_is_completed(master_cp))
	{
	    fstate_src_ty   *src_data;

	    src_data =
		change_file_find(master_cp, the_file_name, view_path_first);
	    if (src_data)
	    {
		const char      *html_class;

		html_class = (((num / 3) & 1) ?  "even-group" : "odd-group");
		printf("<tr class=\"%s\">", html_class);

		// filename column
		printf("<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>\n");

		// usage column
		printf("<td valign=\"top\">\n");
		if (usage_track != src_data->usage)
		    printf("%s", file_usage_ename(src_data->usage));
		printf("</td>\n");

		// action column
		printf("<td valign=\"top\">\n");
		if (action_track != src_data->action)
		    printf("%s", file_action_ename(src_data->action));
		printf("</td>\n");

		// delta column
		printf("<td valign=\"top\">\n");
		emit_change_href(master_cp, "menu");
		html_encode_string(change_version_get(master_cp));
		printf("</a></td>\n");

		// date and time column
		printf("<td></td>\n");

		// change column
		printf("<td valign=\"top\" align=\"right\">\n");
		emit_change_href(master_cp, "menu");
		printf("%ld", magic_zero_decode(master_cp->number));
		printf("</a></td>\n");

		// description column
		printf("<td valign=\"top\">\n");
		emit_change_brief_description(master_cp);
		printf("</td>\n");

		// download column
		printf("<td valign=\"top\">");
		emit_change_href(master_cp, "download");
		printf("Download</a></td></tr>\n");
	    }
	}
    }
    printf("<tr class=\"even-group\"><td colspan=8>");
    printf("Listed %ld file%s.", (long)num_files, (num_files == 1 ? "" : "s"));
    if (!all && !detailed)
    {
	printf("  There is also a ");
	emit_file_href(master_cp, filename, "history+detailed");
	printf("detailed</a> version of this listing available.\n");
    }
    printf("</td></tr>\n");

    printf("</table>\n");
    if (!all && !detailed)
    {
	printf("<p>\nThere is also a ");
	emit_file_href(master_cp, filename, "history+detailed");
	printf("detailed</a> version of this listing available.\n");
    }
    printf("</div>\n");
    html_footer(pp, master_cp);
    trace(("}\n"));
}
