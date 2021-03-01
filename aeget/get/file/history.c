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
 * MANIFEST: functions to manipulate historys
 */

#include <ac/stdio.h>
#include <ac/string.h>

#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <cstate.h>
#include <emit/edit_number.h>
#include <error.h> /* for assert */
#include <get/file/history.h>
#include <http.h>
#include <project.h>
#include <project/file/roll_forward.h>
#include <str_list.h>
#include <trace.h>


static string_ty *
change_brief_description_get(change_ty *cp)
{
    cstate_ty       *cstate_data;

    cstate_data = change_cstate_get(cp);
    return cstate_data->brief_description;
}


void
get_file_history(change_ty *master_cp, string_ty *filename,
    string_list_ty *modifier)
{
    int             all;
    project_ty      *pp;
    time_t          when;
    int             detailed = 0;
    string_list_ty  key;
    size_t          num_files;
    size_t          j;

    trace(("get_file_history()\n{\n"));
    all = 0;
    if (0 == strcmp(filename->str_text, "."))
	all = 1;
    for (j = 0; j < modifier->nstrings; ++j)
	if (0 == strcasecmp(modifier->string[j]->str_text, "detailed"))
	    detailed = 1;

    /*
     * Reconstruct the project file history.
     */
    when = change_completion_timestamp(master_cp);
    pp = master_cp->pp;
    project_file_roll_forward(pp, when, detailed);
    project_file_roll_forward_keys(&key);

    html_header(pp);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(", File History</title></head>\n<body><h1 align=center>");
    emit_change(master_cp);
    printf(",<br>\nFile History</h1>\n");

    printf("<div class=\"information\"><table align=center>\n");
    printf("<tr class=\"even-group\"><th colspan=3>File Name</th>\n");
    printf("<th>Delta</th><th>Date and Time</th><th>Edit</th>\n");
    printf("<th>Description</th><th>&nbsp;</th></tr>\n");

    /*
     * list the change's files' histories
     */
    num_files = 0;
    for (j = 0; j < key.nstrings; ++j)
    {
	file_event_list_ty *felp;
	size_t		k;
	int		usage_track;
	int		action_track;
	string_ty       *the_file_name;
	size_t          num;

	the_file_name = key.string[j];
	if (!all && !str_equal(filename, the_file_name))
	    continue;
	if (!master_cp->bogus && !change_file_find(master_cp, the_file_name))
	    continue;
	++num_files;

	printf("<tr class=\"even-group\"><td colspan=8>\n");
	emit_file_href(master_cp, the_file_name, "menu");
	html_encode_string(the_file_name);
	printf("</td></tr>\n");
	num = 0;

	usage_track = -1;
	action_track = -1;

	felp = project_file_roll_forward_get(the_file_name);
	if (felp)
	{
	    for (k = 0; k < felp->length; ++k)
	    {
		file_event_ty	*fep;
		fstate_src_ty   *src2_data;
		string_ty	*s;
		const char      *html_class;

		fep = felp->item + k;
		src2_data = change_file_find(fep->cp, the_file_name);
		assert(src2_data);
		if (!src2_data)
		    continue;

		html_class = (((num / 3) & 1) ?  "even-group" : "odd-group");
		++num;
		printf("<tr class=\"%s\">", html_class);

		/* filename column */
		printf("<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>\n");

		/* usage column */
		printf("<td valign=\"top\">");
		if (usage_track != src2_data->usage)
		{
		    printf("%s", file_usage_ename(src2_data->usage));
		    usage_track = src2_data->usage;
		}
		printf("</td>\n");

		/* action column */
		printf("<td valign=\"top\">");
		if (action_track != src2_data->action)
		{
		    printf("%s", file_action_ename(src2_data->action));
		    action_track = src2_data->action;
		}
		printf("</td>\n");

		/* delta column */
		printf("<td valign=\"top\">");
		emit_change_href(fep->cp, "menu");
		s = change_version_get(fep->cp);
		html_encode_string(s);
		str_free(s);
		printf("</a></td>\n");

		/* date and time column */
		printf("<td valign=\"top\">");
		html_encode_charstar(ctime(&fep->when));
		printf("</td>\n");

		/* change column */
		printf("<td valign=\"top\" align=\"right\">");
		emit_file_href(fep->cp, src2_data->file_name, 0);
		emit_edit_number(fep->cp, src2_data);
		printf("</a></td>\n");

		/* description column */
		printf("<td valign=\"top\">\n");
		html_encode_string(change_brief_description_get(fep->cp));
		printf("</td>\n");

		/* download column */
		printf("<td valign=\"top\">");
		emit_change_href(fep->cp, "download");
		printf("Download</a></td></tr>\n");
	    }
	}

	/*
	 * Now output details of this change, as the "end"
	 * of the history.
	 */
	if (!master_cp->bogus && !change_is_completed(master_cp))
	{
	    fstate_src_ty   *src_data;

	    src_data = change_file_find(master_cp, the_file_name);
	    if (src_data)
	    {
		const char      *html_class;

		html_class = (((num / 3) & 1) ?  "even-group" : "odd-group");
		printf("<tr class=\"%s\">", html_class);

		/* filename column */
		printf("<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>\n");

		/* usage column */
		printf("<td valign=\"top\">\n");
		if (usage_track != src_data->usage)
		    printf("%s", file_usage_ename(src_data->usage));
		printf("</td>\n");

		/* action column */
		printf("<td valign=\"top\">\n");
		if (action_track != src_data->action)
		    printf("%s", file_action_ename(src_data->action));
		printf("</td>\n");

		/* delta column */
		printf("<td valign=\"top\">\n");
		emit_change_href(master_cp, "menu");
		html_encode_string(change_version_get(master_cp));
		printf("</a></td>\n");

		/* date and time column */
		printf("<td></td>\n");

		/* change column */
		printf("<td valign=\"top\" align=\"right\">\n");
		emit_change_href(master_cp, "menu");
		printf("%ld", magic_zero_decode(master_cp->number));
		printf("</a></td>\n");

		/* description column */
		printf("<td valign=\"top\">\n");
		html_encode_string(change_brief_description_get(master_cp));
		printf("</td>\n");

		/* download column */
		printf("<td valign=\"top\">");
		emit_change_href(master_cp, "download");
		printf("Download</a></td></tr>\n");
	    }
	}
    }
    printf("<tr class=\"odd-group\"><td colspan=8>Listed %ld", (long)num_files);
    printf(" file%s.</td></tr>\n", (num_files == 1 ? "" : "s"));

    printf("</table></div>\n");
    html_footer();
    trace(("}\n"));
}
