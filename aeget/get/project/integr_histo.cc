//
//      aegis - project change supervisor
//      Copyright (C) 2004-2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program.  If not, see
//      <http://www.gnu.org/licenses/>,
//

#include <common/ac/string.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/gettime.h>
#include <common/now.h>
#include <common/nstring.h>
#include <common/str_list.h>
#include <libaegis/emit/project.h>
#include <libaegis/http.h>
#include <libaegis/project.h>
#include <libaegis/project/change/walk.h>

#include <aeget/change/functor/integr_histo.h>
#include <aeget/get/project/integr_histo.h>


static const char *const month_name[] =
{
    "January", "February", "March", "April", "May", "June", "July",
    "August", "September", "October", "November", "December"
};


void
get_project_integration_histogram(project *pp, string_ty *,
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
    bool recursive = modifier_test_and_clear(modifier_p, "recursive");
    const char *plus_recursive = "";
    if (recursive)
        plus_recursive = "+recursive";

    const char *format = "%Y";
    time_t earliest = 0;
    time_t latest = now();
    if (year)
    {
        if (month)
        {
            nstring when =
                nstring::format
                (
                    "1-%3.3s-%04d 00:00:00",
                    month_name[month - 1],
                    year
                );
            earliest = date_scan(when.c_str());
            int y2 = year;
            int m2 = month + 1;
            if (m2 > 12)
            {
                y2++;
                m2 = 1;
            }
            when =
                nstring::format
                (
                    "1-%3.3s-%04d 00:00:00",
                    month_name[m2 - 1],
                    y2
                );
            latest = date_scan(when.c_str()) - 1;
            format = "%d";
        }
        else
        {
            nstring when = nstring::format("1-Jan-%04d 00:00:00", year);
            earliest = date_scan(when.c_str());
            when = nstring::format("1-Jan-%04d 00:00:00", year + 1);
            latest = date_scan(when.c_str()) - 1;
            format = "%m";
        }
    }

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
    {
        emit_project_href
        (
            pp,
            "project+integration-histogram%s",
            plus_recursive
        );
    }
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
                "project+integration-histogram+year=%d%s",
                year,
                plus_recursive
            );
        }
        printf("%d", year);
        if (month)
            printf("</a>");
    }
    printf("</h1>\n");
    printf("<div class=\"information\">\n");

    change_functor_integration_histogram
        cfih(pp, earliest, latest, recursive, format);
    project_change_walk(pp, cfih);
    cfih.print();

    printf("<p align=\"center\">There is also a ");
    nstring ref = "project+integration-histogram";
    if (year)
    {
        ref += nstring::format("+year=%d", year);
        if (month)
            ref += nstring::format("+month=%d", month);
    }
    if (recursive)
    {
        printf("faster ");
        emit_project_href(pp, "%s", ref.c_str());
        printf("non-");
    }
    else
    {
        printf("slower ");
        ref += "+recursive";
        emit_project_href(pp, "%s", ref.c_str());
    }
    printf("recursive</a> listing available.</p>\n");

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


// vim: set ts=8 sw=4 et :
