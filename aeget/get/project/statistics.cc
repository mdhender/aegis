//
//      aegis - project change supervisor
//      Copyright (C) 2003-2008, 2011, 2012 Peter Miller
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
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/stdio.h>

#include <common/now.h>
#include <common/str_list.h>
#include <libaegis/aer/func/now.h> // working_days
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/emit/project.h>
#include <libaegis/fstate.fmtgen.h>
#include <libaegis/http.h>
#include <libaegis/project.h>

#include <aeget/get/project/statistics.h>


void
get_project_statistics(project *pp, string_ty *, string_list_ty *)
{
    double          scale;
    cstate_branch_ty *bp;
    change::pointer cp;
    long            cause_stats[change_cause_max];
    long            number_of_changes = 0;
    long            file_action_stats[file_action_max];
    long            file_action_total = 0;
    long            file_usage_stats[file_usage_max];
    long            file_usage_total = 0;
    double          hist_max[cstate_state_max];
    double          hist_min[cstate_state_max];
    double          hist_stats[cstate_state_max];
    double          hist_total = 0;
    double          hist_total_max = 0;
    double          hist_total_min = 0;
    size_t          n;
    cstate_ty       *cstate_data;

    html_header(pp, 0);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(" Statistics</title></head><body>\n");
    html_header_ps(pp, 0);
    printf("<h1 align=center>\n");
    emit_project(pp);
    printf(",<br>\nStatistics</h1>\n");
    printf("<div class=\"information\">\n");

    printf("This page provides a number of statistical projections of\n");
    printf("the audit data collected as changes progress through the\n");
    printf("development process.\n");

    for (n = 0; n < change_cause_max; ++n)
        cause_stats[n] = 0;
    for (n = 0; n < file_action_max; ++n)
        file_action_stats[n] = 0;
    for (n = 0; n < file_usage_max; ++n)
        file_usage_stats[n] = 0;
    for (n = 0; n < cstate_state_max; ++n)
    {
        hist_stats[n] = 0;
        hist_min[n] = 0;
        hist_max[n] = 0;
    }

    //
    // traverse each change
    //
    cp = pp->change_get();
    cstate_data = cp->cstate_get();
    bp = cstate_data->branch;
    assert(bp);
    if (!bp->change)
    {
        bp->change =
            (cstate_branch_change_list_ty *)
            cstate_branch_change_list_type.alloc();
    }
    for (n = 0; n < bp->change->length; ++n)
    {
        long            change_number;
        int             nn;
        cstate_state_ty hist_state;
        time_t          hist_time;
        double          j;
        double          hist_chan[cstate_state_max];

        change_number = bp->change->list[n];
        cp = change_alloc(pp, change_number);
        change_bind_existing(cp);
        cstate_data = cp->cstate_get();
        cause_stats[cstate_data->cause]++;
        number_of_changes++;

        for (nn = 0; ; ++nn)
        {
            fstate_src_ty   *src;

            src = change_file_nth(cp, nn, view_path_first);
            if (!src)
                break;

            file_action_stats[src->action]++;
            file_action_total++;
            switch (src->usage)
            {
            case file_usage_source:
            case file_usage_config:
            case file_usage_test:
            case file_usage_manual_test:
                break;

            case file_usage_build:
                switch (src->action)
                {
                case file_action_modify:
                    continue;

                case file_action_create:
                case file_action_remove:
                case file_action_insulate:
                case file_action_transparent:
                    break;
                }
                break;
            }
            file_usage_stats[src->usage]++;
            file_usage_total++;
        }

        hist_state = cstate_state_awaiting_development;
        hist_time = now();
        for (nn = 0; nn < (int)cstate_state_max; ++nn)
            hist_chan[nn] = 0;
        assert(cstate_data->history);
        for (nn = 0; nn < (int)cstate_data->history->length; ++nn)
        {
            cstate_history_ty *hp;

            hp = cstate_data->history->list[nn];
            if (hp->what == cstate_history_what_new_change)
            {
                hist_time = hp->when;
                hist_state = cstate_state_awaiting_development;
                continue;
            }
            hist_chan[hist_state] += working_days(hist_time, hp->when);
            hist_time = hp->when;
            switch (hp->what)
            {
            case cstate_history_what_new_change:
            case cstate_history_what_develop_begin_undo:
                hist_state = cstate_state_awaiting_development;
                break;

            case cstate_history_what_develop_begin:
            case cstate_history_what_develop_end_undo:
            case cstate_history_what_integrate_fail:
            case cstate_history_what_review_fail:
                hist_state = cstate_state_being_developed;
                break;

            case cstate_history_what_develop_end_2ar:
            case cstate_history_what_review_begin_undo:
            case cstate_history_what_review_pass_2ar:
            case cstate_history_what_review_pass_undo_2ar:
                hist_state = cstate_state_awaiting_review;
                break;

            case cstate_history_what_develop_end:
            case cstate_history_what_review_begin:
            case cstate_history_what_review_pass_undo:
            case cstate_history_what_review_pass_2br:
                hist_state = cstate_state_being_reviewed;
                break;

            case cstate_history_what_develop_end_2ai:
            case cstate_history_what_review_pass:
            case cstate_history_what_integrate_begin_undo:
                hist_state = cstate_state_awaiting_integration;
                break;

            case cstate_history_what_integrate_begin:
                hist_state = cstate_state_being_integrated;
                break;

            case cstate_history_what_integrate_pass:
                hist_state = cstate_state_completed;
                break;
            }
        }
        if (hist_state != cstate_state_completed)
        {
            hist_chan[hist_state] += working_days(hist_time, now());
            hist_time = now();
        }

        for (nn = 0; nn < cstate_state_max; ++nn)
        {
            j = hist_chan[nn];
            hist_stats[nn] += j;
            if (hist_min[nn] == 0 || j < hist_min[nn])
                hist_min[nn] = j;
            if (j > hist_max[nn])
                hist_max[nn] = j;
        }
        j = working_days(cstate_data->history->list[0]->when, hist_time);
        hist_total += j;
        if (!hist_total_min || j < hist_total_min)
            hist_total_min = j;
        if (j > hist_total_max)
            hist_total_max = j;
    }
    printf("There were %ld changes to this project.\n", number_of_changes);

    //
    // print statistics about change state durations
    //
    printf("<hr>\n");
    printf("<h2>Change Duration By State</h2>\n");
    printf("<table align=center>\n");
    printf("<tr><th>Change State</th><th>Working Days</th><th>Minimum</th>");
    printf("<th>Average</th><th>Maximum</th></tr>\n");
    scale = 0;
    for (n = 0; n < cstate_state_max; ++n)
    {
        double          hist_time;

        hist_time = hist_stats[n];
        if (hist_time > scale)
            scale = hist_time;
    }
    if (scale == 0)
        scale = 1;
    else
        scale = HISTOGRAM_WIDTH / scale;
    for (n = 0; n < cstate_state_max; ++n)
    {
        double          hist_time;

        if (hist_max[n] == 0)
            continue;
        hist_time = hist_stats[n];
        printf("<tr><td valign=top>\n");
        printf("%s\n", cstate_state_ename((cstate_state_ty)n));
        printf("</td><td valign=top align=right>\n");
        printf("%.2f\n", hist_time);
        printf("</td><td valign=top align=right>\n");
        printf("%.2f\n", hist_min[n]);
        printf("</td><td valign=top align=right>\n");
        printf("%.2f\n", hist_time / number_of_changes);
        printf("</td><td valign=top align=right>\n");
        printf("%.2f\n", hist_max[n]);
        if (hist_time > 0)
        {
            int             width;

            width = (int)(0.5 + hist_time * scale);
            printf("</td><td>\n");
            emit_rect_image(width, HISTOGRAM_HEIGHT, 0);
        }
        printf("</td></tr>\n");
    }
    printf("<tr><td></td><td><hr></td><td><hr></td><td><hr></td>");
    printf("<td><hr></td></tr>");
    printf("<tr><td valign=top>Total");
    printf("</td><td valign=top align=right>\n");
    printf("%.2f", hist_total);
    printf("</td><td valign=top align=right>\n");
    printf("%.2f", hist_total_min);
    printf("</td><td valign=top align=right>\n");
    printf("%.2f\n", hist_total / number_of_changes);
    printf("</td><td valign=top align=right>\n");
    printf("%.2f\n", hist_total_max);
    printf("</td></tr>\n");
    printf("</table>\n");

    //
    // print statistics about change causes
    //
    printf("<hr>\n");
    printf("<h2>Change Cause Distribution</h2>\n");
    printf("<table align=center>\n");
    printf("<tr><th>Change Cause</th><th>Count</th><th>Percent</th></tr>\n");
    scale = 0;
    for (n = 0; n < change_cause_max; ++n)
    {
        if (cause_stats[n] > scale)
            scale = cause_stats[n];
    }
    if (scale == 0)
        scale = 1;
    else
        scale = HISTOGRAM_WIDTH / scale;
    for (n = 0; n < change_cause_max; ++n)
    {
        printf("<tr><td valign=top>\n");
        printf("%s\n", change_cause_ename((change_cause_ty)n));
        printf("</td><td valign=top align=right>\n");
        printf("%ld\n", cause_stats[n]);
        printf("</td><td valign=top align=right>\n");
        printf("%.2f%%", 100. * cause_stats[n] / number_of_changes);
        if (cause_stats[n] > 0)
        {
            int             width;

            width = (int)(0.5 + cause_stats[n] * scale);
            printf("</td><td>\n");
            emit_rect_image(width, HISTOGRAM_HEIGHT, 0);
        }
        printf("</td></tr>\n");
    }
    printf("<tr><td></td><td><hr></td><td></td></tr>\n");
    printf("<tr><td valign=top>Total\n");
    printf("</td><td valign=top align=right>\n");
    printf("%ld\n", number_of_changes);
    printf("</td><td valign=top align=right>\n");
    printf("</td></tr>\n");
    printf("</table>\n");

    //
    // print statistics about file actions
    //
    printf("<hr>\n");
    printf("<h2>Change File Action Distribution</h2>\n");
    printf("<table align=center>\n");
    printf("<tr><th>Action</th><th>Count</th>\n");
    printf("<th>Percent</th><th>Average</th></tr>\n");
    scale = 0;
    for (n = 0; n < file_action_max; ++n)
    {
        if (file_action_stats[n] > scale)
            scale = file_action_stats[n];
    }
    if (scale == 0)
        scale = 1;
    else
        scale = HISTOGRAM_WIDTH / scale;
    for (n = 0; n < file_action_max; ++n)
    {
        printf("<tr><td valign=top>\n");
        printf("%s\n", file_action_ename((file_action_ty)n));
        printf("</td><td valign=top align=right>\n");
        printf("%ld\n", file_action_stats[n]);
        printf("</td><td valign=top align=right>\n");
        printf("%.2f%%\n", 100. * file_action_stats[n] / file_action_total);
        printf("</td><td valign=top align=right>\n");
        printf("%.2f\n", (double)file_action_stats[n] / number_of_changes);
        if (file_action_stats[n] > 0)
        {
            int             width;

            width = (int)(0.5 + file_action_stats[n] * scale);
            printf("</td><td>\n");
            emit_rect_image(width, HISTOGRAM_HEIGHT, 0);
        }
        printf("</td></tr>\n");
    }
    printf("<tr><td></td><td><hr></td><td></td><td><hr></td></tr>\n");
    printf("<tr><td valign=top>Total\n");
    printf("</td><td valign=top align=right>\n");
    printf("%ld\n", file_action_total),
    printf("</td><td></td><td valign=top align=right>\n");
    printf("%.2f\n", (double)file_action_total / number_of_changes);
    printf("</td></tr>\n");
    printf("</table>\n");

    //
    // print statistics about file usages
    //
    printf("<hr>\n");
    printf("<h2>Change File Type Distribution</h2>\n");
    printf("<table align=center>\n");
    printf("<tr><th>Type</th><th>Count</th><th>Percent</th>");
    printf("<th>Average</th></tr>\n");
    scale = 0;
    for (n = 0; n < file_usage_max; ++n)
    {
        if (file_usage_stats[n] > scale)
            scale = file_usage_stats[n];
    }
    if (scale == 0)
        scale = 1;
    else
        scale = HISTOGRAM_WIDTH / scale;
    for (n = 0; n < file_usage_max; ++n)
    {
        printf("<tr><td valign=top>\n");
        printf("%s\n", file_usage_ename((file_usage_ty)n));
        printf("</td><td valign=top align=right>\n");
        printf("%ld\n", file_usage_stats[n]);
        printf("</td><td valign=top align=right>\n");
        printf("%.2f%%\n", 100. * file_usage_stats[n] / file_usage_total);
        printf("</td><td valign=top align=right>\n");
        printf("%.2f\n", (double)file_usage_stats[n] / number_of_changes);
        if (file_usage_stats[n] > 0)
        {
            int             width;

            width = (int)(0.5 + file_usage_stats[n] * scale);
            printf("</td><td>\n");
            emit_rect_image(width, HISTOGRAM_HEIGHT, 0);
        }
        printf("</td></tr>\n");
    }
    printf("<tr><td></td><td><hr></td><td></td><td><hr></td></tr>\n");
    printf("<tr><td valign=top>Total\n");
    printf("</td><td valign=top align=right>\n");
    printf("%ld\n", file_usage_total);
    printf("</td><td></td><td valign=top align=right>\n");
    printf("%.2f\n", (double)file_usage_total / number_of_changes);
    printf("</td></tr>\n");
    printf("</table>\n");
    printf("</div>");

    printf("<hr>\n");
    printf("<div class=\"report-cmd\">\n");
    printf("A similar report may be obtained from the command line, with\n");
    printf("<blockquote><pre>aer proj_stats -p ");
    html_encode_string(project_name_get(pp));
    printf("</pre></blockquote>\n");
    printf("</div>\n");

    printf("<hr>\n");
    printf("<p align=center class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(pp, "menu");
    printf("Project Menu</a>\n");
    printf("]</p>\n");

    html_footer(pp, 0);
}


// vim: set ts=8 sw=4 et :
