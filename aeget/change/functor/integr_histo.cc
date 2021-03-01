//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/yyyymmdd_wk.h>
#include <libaegis/change/branch.h>
#include <libaegis/http.h>

#include <aeget/change/functor/integr_histo.h>


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



change_functor_integration_histogram::~change_functor_integration_histogram()
{
}


change_functor_integration_histogram::change_functor_integration_histogram(
	project_ty *arg1, time_t arg2, time_t arg3, bool a_recurse,
	const char *arg5) :
    change_functor(!a_recurse, false),
    pp(arg1),
    min_time(arg2),
    max_time(arg3),
    recurse(a_recurse),
    format(arg5),
    size_of_biggest_bucket(0),
    min_seen(0),
    max_seen(0)
{
    bucket.set_reap(reap_long);
}


time_t
change_functor_integration_histogram::earliest()
{
    return min_time;
}


time_t
change_functor_integration_histogram::latest()
{
    return max_time;
}


bool
change_functor_integration_histogram::recurse_branches()
{
    return recurse;
}


void
change_functor_integration_histogram::operator()(change::pointer cp)
{
    //
    // We are only interested in completed change sets.
    //
    assert(cp->is_completed());
    time_t t = change_completion_timestamp(cp);

    //
    // Track domain.
    //
    if (min_seen == 0 && max_seen == 0)
    {
	min_seen = t;
	max_seen = t;
    }
    else
    {
	if (min_seen > t)
	    min_seen = t;
	if (max_seen < t)
	    max_seen = t;
    }

    //
    // Build a key into our table.
    //
    struct tm *tmp = localtime(&t);
    char buffer[20];
    strftime(buffer, sizeof(buffer), format, tmp);
    string_ty *key = str_from_c(buffer);

    //
    // Add the data to the table.
    //
    long *p = (long *)bucket.query(key);
    if (!p)
    {
	p = new long(0);
	bucket.assign(key, p);
    }
    ++*p;

    //
    // Track range.
    //
    if (size_of_biggest_bucket < *p)
	size_of_biggest_bucket = *p;
    str_free(key);
}


static int
year_of(time_t when)
{
    struct tm *tmp = localtime(&when);
    return (1900 + tmp->tm_year);
}


void
change_functor_integration_histogram::print()
{
    if (min_seen == 0 && max_seen == 0)
    {
	printf("No changes in this date range.\n");
	return;
    }

    double dmax = 400. / size_of_biggest_bucket;

    long total = 0;
    printf("<table align=center>\n");
    const char *plus_recursive = "";
    if (recurse)
	plus_recursive = "+recursive";
    if (format[1] == 'Y')
    {
	int min_year = year_of(min_seen);
	int max_year = year_of(max_seen);

	printf("<tr class=\"even-group\">");
	printf("<th>Year</th><th>Count</th><th>&nbsp;</th></tr>\n");
	int num = 0;
	for (int year = min_year; year <= max_year; ++year)
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
		    "project+integration-histogram+year=%d%s",
		    year,
		    plus_recursive
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
	    else
		printf("&nbsp;");
	    printf("</td></tr>\n");
	}
    }
    else if (format[1] == 'm')
    {
	int year = year_of(min_seen);
	printf("<tr class=\"even-group\">");
	printf("<th>Month</th><th>Count</th><th>&nbsp;</th></tr>\n");
	int num = 0;
	for (int month = 1; month <= 12; ++month)
	{
	    string_ty *key = str_format("%2.2d", month);
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
		    "project+integration-histogram+year=%d+month=%d%s",
		    year,
		    month,
		    plus_recursive
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
	    else
		printf("&nbsp;");
	    printf("</td></tr>\n");
	}
    }
    else
    {
	struct tm *tmp = localtime(&min_seen);
	int year = 1900 + tmp->tm_year;
	int month = 1 + tmp->tm_mon;
	printf("<tr class=\"even-group\">");
	printf("<th>Day</th><th>Count</th><th>&nbsp;</th></tr>\n");
	int wday = yyyymmdd_to_wday(year, month, 1);
	int maxday = days_in_month(year, month);
	for (int day = 1; day <= maxday; ++day)
	{
	    string_ty *key = str_format("%2.2d", day);
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
	    else
		printf("&nbsp;");
	    printf("</td></tr>\n");
	}
    }
    printf("<tr class=\"even-group\"><td align=right>Total:</td>");
    printf("<td align=right>%ld</td><td>&nbsp;</td></tr>\n", total);
    printf("</table>\n");
}
