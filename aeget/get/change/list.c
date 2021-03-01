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
 * MANIFEST: functions to manipulate lists
 */

#include <ac/stdio.h>

#include <change.h>
#include <cstate.h>
#include <get/change/list.h>
#include <emit/project.h>
#include <http.h>
#include <project.h>
#include <project/history.h>
#include <str_list.h>


static int
calculate_state_mask(string_list_ty *modifier)
{
    int             state_mask;
    string_ty       *not;
    int             invert;
    size_t          j;

    /*
     * The modifiers select the states we want to view.
     * No modifiers means all states.
     */
    state_mask = 0;
    not = str_from_c("not");
    invert = 0;
    for (j = 1; j < modifier->nstrings; ++j)
    {
	int             bit;

	bit = cstate_state_type.enum_parse(modifier->string[j]);
	if (bit >= 0)
	    state_mask |= (1 << bit);
	else if (str_equal(not, modifier->string[j]))
	    invert = 1;
    }
    if (invert)
	state_mask = ~state_mask;
    if (state_mask == 0)
	state_mask = ~0;
    str_free(not);

    return state_mask;
}


static int
single_bit(int n)
{
    int		result;

    /* see if no bits are set */
    if (!n)
	return -1;

    /*
     * see if more than 1 bit is set
     *	(only works on 2s compliment machines)
     */
    if ((n & -n) != n)
	return -1;

    /*
     * will need to extend this for 64bit machines,
     * if ever have >32 states
     */
    result = 0;
    if (n & 0xFFFF0000)
	result += 16;
    if (n & 0xFF00FF00)
	result += 8;
    if (n & 0xF0F0F0F0)
	result += 4;
    if (n & 0xCCCCCCCC)
	result += 2;
    if (n & 0xAAAAAAAA)
	result++;
    return result;
}


void
get_change_list(project_ty *pp, string_ty *filename, string_list_ty *modifier)
{
    int             state_mask;
    int	            bit;
    size_t          j;
    size_t          num;
    int             link_to_state_page;

    /*
     * The modifiers select the states we want to view.
     * No modifiers means all states.
     */
    state_mask = calculate_state_mask(modifier);

    html_header(pp);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(",\nList of Changes</title></head>\n<body><h1 align=center>");
    emit_project(pp);
    printf(",<br>\nList of Changes");
    bit = single_bit(state_mask);
    link_to_state_page = 1;
    if (bit >= 0)
    {
	printf(" %s", cstate_state_ename(bit));
	link_to_state_page = 0;
    }
    else
    {
	bit = single_bit(~state_mask);
	if (bit >= 0)
	    printf(" not %s", cstate_state_ename(bit));
    }
    printf("</h1>\n");

    /*
     * list the project's changes
     */
    printf("<div class=\"information\"><table align=\"center\">\n");
    printf("<tr class=\"even-group\"><th>Change</th><th>State</th>");
    printf("<th>Description</th><th>&nbsp;</ht></tr>\n");
    num = 0;
    for (j = 0; ; ++j)
    {
	cstate          cstate_data;
	long            change_number;
	change_ty       *cp;

	if (!project_change_nth(pp, j, &change_number))
    	    break;
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
	cstate_data = change_cstate_get(cp);
	if (!(state_mask & (1 << cstate_data->state)))
	    continue;
	++num;
	printf("<tr class=\"%s-group\">", (((j / 3) & 1) ? "even" : "odd"));
	printf("<td valign=\"top\" align=\"right\">\n");
	emit_change_href(cp, "menu");
	printf("%ld</a>", change_number);
	printf("</td>\n<td valign=\"top\">\n");
	if (link_to_state_page)
	{
	    printf("<a href=\"%s/", http_script_name());
	    html_escape_string(project_name_get(cp->pp));
	    printf("/@@changes@%s\">", cstate_state_ename(cstate_data->state));
	}
	html_encode_charstar(cstate_state_ename(cstate_data->state));
	if (link_to_state_page)
	    printf("</a>");
	if (cstate_data->state == cstate_state_being_developed)
	{
	    printf("<br>\n<i>");
	    html_encode_string(change_developer_name(cp));
	    printf("</i>");
	}
	if (cstate_data->state == cstate_state_being_integrated)
	{
	    printf("<br>\n<i>");
	    html_encode_string(change_integrator_name(cp));
	    printf("</i>");
	}
	printf("</td>\n<td valign=\"top\">\n");
	if (cstate_data->brief_description)
	    html_encode_string(cstate_data->brief_description);
	printf("</td>\n<td valign=top>\n");
	emit_change_href(cp, "download");
	printf("Download</a>\n");
	printf("</td></tr>\n");
    }
    printf("<tr class=\"even-group\"><td colspan=4>\n");
    printf("Listed %lu changes.</td></tr>\n", (unsigned long)num);
    printf("</table></div>\n");

    printf("<hr>\n");
    printf("A similar report may be obtained from the command line, with\n");
    printf("<blockquote><pre>ael c -p ");
    html_encode_string(project_name_get(pp));
    printf("</pre></blockquote>\n");

    printf("<hr>\n");
    printf("<p align=\"center\" class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(pp, "menu");
    printf("Project Menu</a>\n");
    printf("]</p>\n");

    html_footer();
}
