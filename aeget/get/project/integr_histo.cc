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
// MANIFEST: implementation of the get_project_integr_histo class
//

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>

#include <change.h>
#include <change/branch.h>
#include <cstate.h>
#include <emit/project.h>
#include <error.h> // for assert
#include <get/project/integr_histo.h>
#include <http.h>
#include <project.h>
#include <str_list.h>
#include <symtab.h>
#include <yyyymmdd_wk.h>

static const char *const month_name[] =
{
    "January", "February", "March", "April", "May", "June", "July",
    "August", "September", "October", "November", "December"
};


static void
reap_long(void *p)
{
    delete (long *)p;
}


void
get_project_integration_histogram(project_ty *pp, string_ty *fn,
    string_list_ty *modifier_p)
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
    // Emit the title portion
    //
    html_header(pp, 0);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(" Integration Histogram");
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
    html_header_ps(pp, 0);

    //
    // emit the heading portion
    //
    printf("<h1 align=center>\n");
    emit_project(pp);
    printf(",<br>\n");
    if (year)
	emit_project_href(pp, "project+integration-histogram");
    printf("Integration Histogram");
    if (year)
    {
	printf("</a>,<br>\n");
	if (month)
	{
	    printf("%s ", month_name[month - 1]);
	    emit_project_href
	    (
		pp,
		"project+integration-histogram+year=%d",
		year
	    );
	}
	printf("%d", year);
	if (month)
	    printf("</a>");
    }
    printf("</h1>\n");
    printf("<div class=\"information\">\n");

    int min_year = 0;
    int max_year = 0;
    symtab_ty bucket;
    bucket.set_reap(reap_long);
    change_ty *pcp = project_change_get(pp);
    cstate_ty *cstate_data = change_cstate_get(pcp);
    assert(cstate_data->branch);
    if (!cstate_data->branch->history)
    {
	cstate_data->branch->history =
	    (cstate_branch_history_list_ty *)cstate_branch_history_type.alloc();
    }
    int max = 0;
    for (size_t k = 0; k < cstate_data->branch->history->length; ++k)
    {
	cstate_branch_history_ty *hp = cstate_data->branch->history->list[k];
	change_ty *cp = change_alloc(pp, hp->change_number);
	change_bind_existing(cp);
	assert(change_is_completed(cp));

	string_ty *key = 0;
	time_t t = change_completion_timestamp(cp);
	struct tm *tmp = localtime(&t);
	if (year)
	{
	    if (year == tmp->tm_year + 1900)
	    {
		if (month)
		{
		    if (month == tmp->tm_mon + 1)
		    {
			key = str_format("%d", tmp->tm_mday);
		    }
		}
		else
		{
		    key = str_format("%d", tmp->tm_mon + 1);
		}
	    }
	}
	else
	{
	    if (min_year == 0 || tmp->tm_year + 1900 < min_year)
		min_year = tmp->tm_year + 1900;
	    if (max_year == 0 || tmp->tm_year + 1900 > max_year)
		max_year = tmp->tm_year + 1900;

	    key = str_format("%d", tmp->tm_year + 1900);
	}

	if (key)
	{
	    long *p = (long *)bucket.query(key);
	    if (!p)
	    {
		p = new long(0);
		bucket.assign(key, p);
	    }
	    str_free(key);
	    ++*p;
	    if (max < *p)
		max = *p;
	}
	change_free(cp);
    }

    double dmax = 400. / max;

    long total = 0;
    printf("<table align=center>\n");
    if (!year)
    {
	printf("<tr class=\"even-group\">");
	printf("<th>Year</th><th>Count</th><th>&nbsp;</th></tr>\n");
	int num = 0;
	for (year = min_year; year <= max_year; ++year)
	{
	    string_ty *key = str_format("%d", year);
	    long *p = (long *)bucket.query(key);
	    str_free(key);
	    int n = p ? *p : 0;
	    total += n;

	    const char *html_class = (((num++ / 3) & 1) ?  "even" : "odd");
	    printf("<tr class=\"%s-group\">", html_class);

	    printf("<td align=right>");
	    if (n > 0)
	    {
		emit_project_href
		(
		    pp,
		    "project+integration-histogram+year=%d",
		    year
		);
		printf("%d</a>", year);
	    }
	    else
		printf("%d", year);
	    printf("</td>\n<td align=right>");
	    printf("%d", n);
	    printf("</td>\n<td>");
	    if (n > 0)
	    {
		int width = (int)(0.5 + n * dmax);
		emit_rect_image(width, HISTOGRAM_HEIGHT, 0);
	    }
	    printf("</td></tr>\n");
	}
    }
    else if (!month)
    {
	printf("<tr class=\"even-group\">");
	printf("<th>Month</th><th>Count</th><th>&nbsp;</th></tr>\n");
	int num = 0;
	for (month = 1; month <= 12; ++month)
	{
	    string_ty *key = str_format("%d", month);
	    long *p = (long *)bucket.query(key);
	    str_free(key);
	    int n = p ? *p : 0;
	    total += n;

	    const char *html_class = (((num / 3) & 1) ?  "even" : "odd");
	    ++num;
	    printf("<tr class=\"%s-group\">", html_class);

	    printf("<td align=right>");
	    if (n > 0)
	    {
		emit_project_href
		(
		    pp,
		    "project+integration-histogram+year=%d+month=%d",
		    year,
		    month
		);
		printf("%s</a>", month_name[month - 1]);
	    }
	    else
		printf("%s", month_name[month - 1]);
	    printf("</td>\n<td align=right>");
	    printf("%d", n);
	    printf("</td>\n<td>");
	    if (n > 0)
	    {
		int width = (int)(0.5 + n * dmax);
		emit_rect_image(width, HISTOGRAM_HEIGHT, 0);
	    }
	    printf("</td></tr>\n");
	}
    }
    else
    {
	printf("<tr class=\"even-group\">");
	printf("<th>Day</th><th>Count</th><th>&nbsp;</th></tr>\n");
	int wday = yyyymmdd_to_wday(year, month, 1);
	int maxday = days_in_month(year, month);
	for (int day = 1; day <= maxday; ++day)
	{
	    string_ty *key = str_format("%d", day);
	    long *p = (long *)bucket.query(key);
	    str_free(key);
	    int n = p ? *p : 0;
	    total += n;

	    bool weekend = (wday == 0 || wday == 6);
	    const char *html_class = (weekend ?  "even" : "odd");
	    if (++wday >= 7)
		wday = 0;
	    printf("<tr class=\"%s-group\">", html_class);

	    printf("<td align=right>");
	    printf("%d", day);
	    printf("</td>\n<td align=right>");
	    printf("%d", n);
	    printf("</td>\n<td>");
	    if (n > 0)
	    {
		int width = (int)(0.5 + n * dmax);
		emit_rect_image(width, HISTOGRAM_HEIGHT, 0);
	    }
	    printf("</td></tr>\n");
	}
    }
    printf("<tr class=\"even-group\"><td align=right>Total:</td>");
    printf("<td align=right>%ld</td><td>&nbsp;</td></tr>\n", total);
    printf("</table>\n");
    printf("</div>\n");

    //
    // Emit the footer portion.
    //
    printf("<hr>\n");
    printf("<p align=center class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(pp, "menu");
    printf("Project Menu</a>");
    if (year)
    {
	printf(" |\n");
	emit_project_href(pp, "project+integration-histogram");
	printf("Integration Histogram</a>");
	if (month)
	{
	    printf(",\n");
	    emit_project_href
	    (
		pp,
		"project+integration-histogram+year=%d",
		year
	    );
	    printf("%d</a>", year);
	}
    }
    printf("\n]</p>\n");

    html_footer(pp, 0);
}
