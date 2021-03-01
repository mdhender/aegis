//
// aegis - project change supervisor
// Copyright (C) 2004-2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/stdlib.h>

#include <common/str_list.h>
#include <common/symtab/template.h>
#include <common/yyyymmdd_wk.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/emit/project.h>
#include <libaegis/http.h>

#include <aeget/get/project/change_cause.h>


static const char *const month_name[] =
{
    "January", "February", "March", "April", "May", "June", "July",
    "August", "September", "October", "November", "December"
};


static const char *
color_map(change_cause_ty x)
{
    switch (x)
    {
    case change_cause_chain:
        return "255,0,0";

    case change_cause_internal_bug:
        return "255,255,0";

    case change_cause_internal_enhancement:
        return "0,255,0";

    case change_cause_internal_improvement:
        return "0,255,255";

    case change_cause_external_bug:
        return "192,192,0";

    case change_cause_external_enhancement:
        return "0,192,0";

    case change_cause_external_improvement:
        return "0,192,192";
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
get_project_change_cause(project *pp, string_ty *,
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
    // Emit page title.
    //
    html_header(pp, 0);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(" Change Cause Histogram");
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
        emit_project_href(pp, "project+change-cause");
    printf("Change Cause Histogram");
    if (year)
    {
        printf("</a>,<br>\n");
        if (month)
        {
            printf("%s ", month_name[month - 1]);
            emit_project_href(pp, "project+change-cause+year=%d", year);
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
    symtab<long> sum_by_cause_by_when[change_cause_max];
    long legend_needed[change_cause_max];
    for (int nn = 0; nn < change_cause_max; ++nn)
        legend_needed[nn] = 0;
    symtab<long> sum_by_when;
    change::pointer pcp = pp->change_get();
    cstate_ty *proj_cstate_data = pcp->cstate_get();
    for (size_t k = 0; k < proj_cstate_data->branch->history->length; ++k)
    {
        cstate_branch_history_ty *hp =
            proj_cstate_data->branch->history->list[k];
        change::pointer cp = change_alloc(pp, hp->change_number);
        change_bind_existing(cp);
        assert(cp->is_completed());
        cstate_ty *cstate_data = cp->cstate_get();
        time_t when = cp->completion_timestamp();
        struct tm *tmp = localtime(&when);
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
        if (!key.empty())
        {
            legend_needed[cstate_data->cause]++;
            symtab<long> *sum_by_when_p =
                &sum_by_cause_by_when[cstate_data->cause];
            incr(sum_by_when_p, key);
            incr(&sum_by_when, key);

            if (earliest_when == 0 || earliest_when > tmp->tm_year + 1900)
                earliest_when = tmp->tm_year + 1900;
            if (latest_when < tmp->tm_year + 1900)
                latest_when = tmp->tm_year + 1900;
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
                emit_project_href(pp, "project+change-cause+year=%d", value);
            }
            else if (!month)
            {
                emit_project_href
                (
                    pp,
                    "project+change-cause+year=%d+month=%d",
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
        // Emit the number of cause of the value.
        //
        printf("<td align=right>%ld</td>\n", n);

        //
        // Emit a scaled rectangles for each cause in that value.
        //
        printf("<td>");
        for (int t = 0; t < change_cause_max; ++t)
        {
            sum_p = sum_by_cause_by_when[t].query(key);
            n = sum_p ? *sum_p : 0;
            const char *clr = color_map((change_cause_ty)t);
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
    printf("</table></div>\n");

    //
    // Emit the legend
    //
    printf("<hr>");
    printf("<div class=\"legend\">");
    printf("<h2 align=center>Legend</h2>");

    printf("<table align=center>");
    for (int t = 0; t < change_cause_max; ++t)
    {
        if (legend_needed[t])
        {
            printf("<tr><td>");
            emit_rect_image_rgb
            (
                30,
                HISTOGRAM_HEIGHT,
                color_map((change_cause_ty)t)
            );
            printf("</td>\n");
            printf
            (
                "<td>%s</td>\n",
                change_cause_ename((change_cause_ty)t)
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
        emit_project_href(pp, "project+change-cause");
        printf("Integration Histogram</a>");
        if (month)
        {
            printf(",\n");
            emit_project_href(pp, "project+change-cause+year=%d", year);
            printf("%d</a>", year);
        }
    }
    printf("]</p>\n");

    //
    // Emit page footer.
    //
    html_footer(pp, 0);
}


// vim: set ts=8 sw=4 et :
