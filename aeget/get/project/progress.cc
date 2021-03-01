//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/stdlib.h>

#include <common/error.h> // for assert
#include <common/str_list.h>
#include <common/symtab/template.h>
#include <common/yyyymmdd_wk.h>
#include <libaegis/change.h>
#include <libaegis/cstate.h>
#include <libaegis/emit/project.h>
#include <libaegis/http.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>

#include <aeget/get/project/progress.h>


static const char *const month_name[] =
{
    "January", "February", "March", "April", "May", "June", "July",
    "August", "September", "October", "November", "December"
};


static const char *
color_map(cstate_history_what_ty x)
{
    switch (x)
    {
    case cstate_history_what_new_change:
	return "128,128,128";

    case cstate_history_what_develop_begin:
	return "0,0,255";

    case cstate_history_what_develop_begin_undo:
	return "0,95,95";

    case cstate_history_what_develop_end:
	return "128,128,255";

    case cstate_history_what_develop_end_2ar:
	return "96,96,255";

    case cstate_history_what_develop_end_2ai:
	return "160,160,255";

    case cstate_history_what_develop_end_undo:
	return "0,255,255";

    case cstate_history_what_review_begin:
	return "255,192,0";

    case cstate_history_what_review_begin_undo:
	return "255,128,0";

    case cstate_history_what_review_pass:
    case cstate_history_what_review_pass_2ar:
    case cstate_history_what_review_pass_2br:
	return "255,255,0";

    case cstate_history_what_review_pass_undo:
    case cstate_history_what_review_pass_undo_2ar:
	return "192,255,64";

    case cstate_history_what_review_fail:
	return "255,0,0";

    case cstate_history_what_integrate_begin:
	return "0,128,0";

    case cstate_history_what_integrate_begin_undo:
	return "0,128,128";

    case cstate_history_what_integrate_fail:
	return "255,0,255";

    case cstate_history_what_integrate_pass:
	return "0,255,0";
    }
    return "64,64,64";
}


static void
incr(symtab<long> *stp, const nstring &key)
{
    long *sum_p = stp->query(key);
    if (!sum_p)
    {
	sum_p = new long(0);
	stp->assign(key, sum_p);
	stp->set_reaper();
    }
    ++*sum_p;
}


void
get_project_progress(project_ty *pp, string_ty *, string_list_ty *modifier_p)
{
    string_list_ty &modifier = *modifier_p;

    //
    // look in the modifiers to find the year and month
    //
    int year = 0;
    int month = 0;
    for (size_t j = 0; j < modifier.size(); ++j)
    {
	string_ty *s = modifier[j];
	if (s->str_length > 5 && 0 == strncasecmp(s->str_text, "year=", 5))
	{
	    year = atoi(s->str_text + 5);
	    if (year < 1970 || year >= 2100)
		year = 0;
	}
	if (s->str_length > 6 && 0 == strncasecmp(s->str_text, "month=", 6))
	{
	    month = atoi(s->str_text + 6);
	    if (month < 1 || month > 12)
		month = 0;
	}
    }
    if (!year)
	month = 0;

    //
    // Emit page title.
    //
    html_header(pp, 0);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf("Progress Histogram");
    if (year)
    {
	printf(", ");
	if (month)
	{
	    printf("%s ", month_name[month - 1]);
	}
	printf("%d", year);
    }
    printf("</title></head><body>\n");

    //
    // Emit page header.
    //
    html_header_ps(pp, 0);
    printf("<h1 align=center>");
    emit_project(pp);
    printf(",<br>\n");
    if (year)
	emit_project_href(pp, "project+progress");
    printf(" Progress Histogram");
    if (year)
    {
	printf("</a>,<br>\n");
	if (month)
	{
	    printf("%s ", month_name[month - 1]);
	    emit_project_href(pp, "project+progress+year=%d", year);
	}
	printf("%d", year);
	if (month)
	    printf("</a>");
    }
    printf("</h1>\n");

    //
    // Scan the changes in this branch.
    //
    int earliest_when = 0;
    int latest_when = 0;
    long rev_pass = 0;
    long rev_fail = 0;
    symtab<long> sum_by_state_transition_by_when[cstate_history_what_max];
    long legend_needed[cstate_history_what_max];
    for (int nn = 0; nn < cstate_history_what_max; ++nn)
	legend_needed[nn] = 0;
    symtab<long> sum_by_when;
    for (size_t m = 0; ; ++m)
    {
	long change_number = 0;
	if (!project_change_nth(pp, m, &change_number))
    	    break;
	change::pointer cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
	cstate_ty *cstate_data = cp->cstate_get();

	assert(cstate_data->history);
	assert(cstate_data->history->length);
	for (size_t k = 0; k < cstate_data->history->length; ++k)
	{
	    cstate_history_ty *hp = cstate_data->history->list[k];
	    struct tm *tmp = localtime(&hp->when);
	    nstring key;
	    if (!year)
		key = nstring::format("%d", tmp->tm_year + 1900);
	    else if (year == tmp->tm_year + 1900)
	    {
		if (!month)
		    key = nstring::format("%d", tmp->tm_mon + 1);
		else if (month == tmp->tm_mon + 1)
		    key = nstring::format("%d", tmp->tm_mday);
	    }
	    if (key.empty())
		continue;
	    legend_needed[hp->what]++;
	    symtab<long> *sum_by_when_p =
		&sum_by_state_transition_by_when[hp->what];
	    incr(sum_by_when_p, key);
	    incr(&sum_by_when, key);

	    if (earliest_when == 0 || earliest_when > tmp->tm_year + 1900)
		earliest_when = tmp->tm_year + 1900;
	    if (latest_when < tmp->tm_year + 1900)
		latest_when = tmp->tm_year + 1900;
	    switch (hp->what)
	    {
	    case cstate_history_what_develop_begin:
	    case cstate_history_what_develop_begin_undo:
	    case cstate_history_what_develop_end:
	    case cstate_history_what_develop_end_2ai:
	    case cstate_history_what_develop_end_2ar:
	    case cstate_history_what_develop_end_undo:
	    case cstate_history_what_integrate_begin:
	    case cstate_history_what_integrate_begin_undo:
	    case cstate_history_what_integrate_fail:
	    case cstate_history_what_integrate_pass:
	    case cstate_history_what_new_change:
	    case cstate_history_what_review_begin:
	    case cstate_history_what_review_begin_undo:
	    case cstate_history_what_review_pass_undo:
	    case cstate_history_what_review_pass_undo_2ar:
		break;

	    case cstate_history_what_review_pass_2ar:
	    case cstate_history_what_review_pass_2br:
		// The transitions don't enter into consideration for
		// the ratio of pass:fail because they aren't complete.
		break;

	    case cstate_history_what_review_pass:
		++rev_pass;
		break;

	    case cstate_history_what_review_fail:
		++rev_fail;
		break;
	    }
	}
	change_free(cp);
    }

    //
    // Emit column headings.
    //
    bool weekends = false;
    printf("<div class=\"information\"><table align=\"center\">\n");
    printf("<tr class=\"even-group\"><th>");
    if (!year)
	printf("Year");
    else if (!month)
    {
	printf("Month");
	earliest_when = 1;
	latest_when = 12;
    }
    else
    {
	printf("Day");
	earliest_when = 1;
	latest_when = days_in_month(year, month);
	weekends = true;
    }
    printf("</th><th>Count</th><th>&nbsp;</ht></tr>\n");

    //
    // Calculate the scale for the histogram.
    //
    int imax = 1;
    for (int ty = earliest_when; ty <= latest_when; ++ty)
    {
	nstring key = nstring::format("%d", ty);
	long *sum_p = sum_by_when.query(key);
	if (sum_p && *sum_p > imax)
	    imax = *sum_p;
    }
    double dmax = 400. / imax;

    //
    // Emit the histogram
    //
    int rownum = (weekends ? yyyymmdd_to_wday(year, month, 1) : 0);
    long total = 0;
    for (int value = earliest_when; value <= latest_when; ++value)
    {
	nstring key = nstring::format("%d", value);
	long *sum_p = sum_by_when.query(key);
	long n = sum_p ? *sum_p : 0;
	total += n;

	//
	// Emit the value.
	//
	const char *html_class = 0;
	if (weekends)
	{
	    bool weekend = (rownum == 0 || rownum == 6);
	    html_class = (weekend ? "even" : "odd");
	    ++rownum;
	    if (rownum >= 7)
		rownum = 0;
	}
	else
	{
	    html_class = (rownum >= 3 ? "even" : "odd");
	    ++rownum;
	    if (rownum >= 6)
		rownum = 0;
	}
	assert(html_class);

	printf("<tr class=\"%s-group\">", html_class);
	printf("<td align=right>");
	if (n > 0)
	{
	    if (!year)
	    {
		emit_project_href(pp, "project+progress+year=%d", value);
	    }
	    else if (!month)
	    {
		emit_project_href
		(
		    pp,
		    "project+progress+year=%d+month=%d",
		    year,
		    value
		);
	    }
	}
	if (year && !month)
	    printf("%s", month_name[value - 1]);
	else
	    printf("%d", value);
	if (n > 0 && (!year || !month))
	    printf("</a>");
	printf("</td>\n");

	//
	// Emit the number of state transitions of the value.
	//
	printf("<td align=right>%ld</td>\n", n);

	//
        // Emit a scaled rectangles for eact type of state transition in
        // that value.
	//
	printf("<td>");
	for (int t = 0; t < cstate_history_what_max; ++t)
	{
	    sum_p = sum_by_state_transition_by_when[t].query(key);
	    n = sum_p ? *sum_p : 0;
	    const char *clr = color_map((cstate_history_what_ty)t);
	    if (n > 0)
	    {
		int width = (int)(0.5 + n * dmax);
		emit_rect_image_rgb(width, HISTOGRAM_HEIGHT, clr, 0);
	    }
	}
	printf("</td></tr>\n");
    }

    //
    // Emit summary row.
    //
    printf("<tr class=\"even-group\"><td colspan=3>\n");
    printf("Listed %lu state transitions.</td></tr>\n", total);
    printf("</table>\n");

    if (rev_fail < rev_pass / 5)
    {
	printf("<p>\n");
	printf("Please note: if the number of <dfn>review_fail</dfn>s is\n");
	if (!rev_fail)
	    printf("zero");
	else
	    printf("relatively low (%ld)", rev_fail);
	printf(" compared to <dfn>review_pass</dfn>es (%ld),\n", rev_pass);
	printf("then the review process may not be working.\n");
	printf("Reviews are supposed to find defects,\n");
	printf("that's why review fails are expected.\n");
	if (!rev_fail)
	{
            printf("You may prefer to set the <dfn>develop\n");
            printf("end action</dfn> to <dfn>goto awaiting\n");
            printf("integration</dfn> in the project attributes.\n");
	}
    }
    printf("</div>\n");

    //
    // Emit the legend
    //
    printf("<hr>");
    printf("<div class=\"legend\">");
    printf("<h2 align=center>Legend</h2>");

    printf("<table align=center>");
    for (int t = 0; t < cstate_history_what_max; ++t)
    {
	if (legend_needed[t])
	{
	    printf("<tr><td>");
	    emit_rect_image_rgb
	    (
		30,
		HISTOGRAM_HEIGHT,
		color_map((cstate_history_what_ty)t)
	    );
	    printf("</td>\n");
	    printf
	    (
		"<td>%s</td>\n",
		cstate_history_what_ename((cstate_history_what_ty)t)
	    );
	    printf("<td align=right>%ld</td></tr>\n", legend_needed[t]);
	}
    }
    printf("</table>");
    printf("</div>");

    //
    // Emit navigation bar.
    //
    printf("<hr>\n");
    printf("<p align=\"center\" class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(pp, "menu");
    printf("Project Menu</a>\n");
    if (year)
    {
	printf(" |\n");
	emit_project_href(pp, "project+progress");
	printf("Integration Histogram</a>");
	if (month)
	{
	    printf(",\n");
	    emit_project_href(pp, "project+progress+year=%d", year);
	    printf("%d</a>", year);
	}
    }
    printf("]</p>\n");

    //
    // Emit page footer.
    //
    html_footer(pp, 0);
}
