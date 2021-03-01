//
// aegis - project change supervisor
// Copyright (C) 2003-2008, 2011, 2012 Peter Miller
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
#include <common/ac/stdlib.h>
#include <common/ac/math.h>

#include <common/mem.h>
#include <common/str_list.h>
#include <common/symtab.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/emit/edit_number.h>
#include <libaegis/emit/project.h>
#include <libaegis/http.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>

#include <aeget/get/file/metrics.h>


struct row_ty
{
    string_ty *filename;
    double data[1];
};


static int
is_an_integer(string_ty *s, long *result_p)
{
    long            result;
    char            *end;

    result = strtol(s->str_text, &end, 10);
    if (*end != 0 || end == s->str_text)
        return 0;
    *result_p = result;
    return 1;
}


static long     sort_col;


static int
row_compare(const void *va, const void *vb)
{
    const row_ty *a;
    const row_ty *b;

    a = *(const row_ty **)va;
    b = *(const row_ty **)vb;
    if (a->data[sort_col - 1] < b->data[sort_col - 1])
        return -1;
    if (a->data[sort_col - 1] > b->data[sort_col - 1])
        return 1;
    return 0;
}


static row_ty *
row_alloc(size_t n)
{
    row_ty          *result;
    size_t          k;

    result = (row_ty *)mem_alloc(sizeof(row_ty) + sizeof(double) * (n - 1));
    result->filename = 0;
    for (k = 0; k < n; ++k)
        result->data[k] = 0;
    return result;
}


void
get_file_metrics(change::pointer cp, string_ty *, string_list_ty *modifier)
{
    project      *pp;
    row_ty          **row;
    row_ty          *sigma_x;
    row_ty          *sigma_x2;
    size_t          j;
    size_t          num_files;
    size_t          total_files;
    string_list_ty  key;

    pp = cp->pp;
    html_header(pp, cp);

    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(", File metrics</title></head><body>\n");
    html_header_ps(pp, cp);
    printf("<h1 align=center>\n");
    emit_project(pp);
    printf(",<br>File metrics</h1>\n");
    printf("<div class=\"information\">\n");

    //
    // Extract the sort column from the modifiers.
    //
    sort_col = 0;
    for (j = 0; j < modifier->nstrings; ++j)
    {
        if (is_an_integer(modifier->string[j], &sort_col))
            break;
    }

    //
    // We make two passes over the project files.  The first pass collects
    // all the names of the availiable metrics.
    //
    symtab_ty *stp = new symtab_ty(8);
    num_files = 0;
    for (j = 0; ; ++j)
    {
        fstate_src_ty   *src;
        metric_list_ty  *mlp;

        if (cp->bogus || !cp->is_completed())
            src = pp->file_nth(j, view_path_extreme);
        else
            src = change_file_nth(cp, j, view_path_first);
        if (!src)
            break;
        mlp = src->metrics;
        if (mlp)
        {
            size_t          k;

            ++num_files;
            for (k = 0; k < mlp->length; ++k)
            {
                static int      yes;
                metric_ty       *mp;

                mp = mlp->list[k];
                stp->assign(mp->name, &yes);
            }
        }
    }

    //
    // Extract the metric names and sort them.
    //
    stp->keys(&key);
    delete stp;
    key.sort();

    row = (row_ty **)mem_alloc(sizeof(row_ty *) * num_files);
    for (j = 0; j < num_files; ++j)
        row[j] = row_alloc(key.nstrings);
    sigma_x = row_alloc(key.nstrings);
    sigma_x2 = row_alloc(key.nstrings);

    //
    // The second pass over the project files to extract the metrics.
    //
    num_files = 0;
    total_files = 0;
    for (j = 0; ; ++j)
    {
        fstate_src_ty   *src;
        metric_list_ty  *mlp;

        if (cp->bogus || !cp->is_completed())
            src = pp->file_nth(j, view_path_extreme);
        else
            src = change_file_nth(cp, j, view_path_first);
        if (!src)
            break;
        mlp = src->metrics;
        if (mlp)
        {
            row_ty          *rp;
            size_t          k;

            rp = row[num_files];
            rp->filename = str_copy(src->file_name);
            for (k = 0; k < mlp->length; ++k)
            {
                metric_ty       *mp;
                size_t          m;

                mp = mlp->list[k];
                for (m = 0; m < key.nstrings; ++m)
                    if (str_equal(key.string[m], mp->name))
                        break;
                rp->data[m] = mp->value;
            }
            ++num_files;
        }
        ++total_files;
    }

    //
    // Sort the rows.
    //
    if (sort_col != 0)
        qsort(row, num_files, sizeof(row[0]), row_compare);

    //
    // Print the column headings.
    //
    printf("<div class=\"information\"><table align=center>\n");
    printf("<tr class=\"even-group\">\n");
    printf("<th>");
    if (sort_col == 0)
        printf("File Name");
    else
    {
        emit_project_href(pp, "file+metrics");
        printf("File Name</a>");
    }
    printf("</th><th>Edit</th>\n");
    for (j = 0; j < key.nstrings; ++j)
    {
        string_list_ty  sl;
        string_ty       *s;
        size_t          k;

        printf("<th>\n");
        if (j + 1 != (size_t)sort_col)
        {
            s = str_format("file+metrics+%d", (int)(j + 1));
            emit_change_href(cp, s->str_text);
            str_free(s);
        }
        sl.split(key.string[j], "_", true);
        if (sl.nstrings)
        {
            s = str_capitalize(sl.string[0]);
            html_encode_string(s);
            str_free(s);
        }
        for (k = 1; k < sl.nstrings; ++k)
        {
            printf("<br>");
            s = str_capitalize(sl.string[k]);
            html_encode_string(s);
            str_free(s);
        }
        if (j + 1 != (size_t)sort_col)
            printf("</a>");
        printf("</th>\n");
    }
    printf("</tr>\n");

    //
    // Print the data.
    //
    for (j = 0; j < num_files; ++j)
    {
        const char      *html_class;
        row_ty          *rp;
        fstate_src_ty   *src;
        size_t          k;

        rp = row[j];
        if (cp->bogus || !cp->is_completed())
            src = pp->file_find(rp->filename, view_path_extreme);
        else
            src = cp->file_find(nstring(rp->filename), view_path_first);
        assert(src);
        html_class = (((j / 3) & 1) ? "even-group" : "odd-group");
        printf("<tr class=\"%s\">\n", html_class);

        printf("<td valign=top>\n");
        printf("<span class=\"filename\">");
        emit_file_href(cp, rp->filename, "menu");
        html_encode_string(rp->filename);
        printf("</span></a>");
        printf("</td>\n");

        printf("<td valign=\"top\" align=\"right\">\n");
        emit_edit_number(cp, src, 0);
        printf("</td>\n");

        for (k = 0; k < key.nstrings; ++k)
        {
            double          x;

            x = rp->data[k];
            printf("<td valign=top align=right>%4.2f</td>\n", x);
            sigma_x->data[k] += x;
            sigma_x2->data[k] += x * x;
        }
        printf("</tr>\n");
    }

    printf("<tr class=\"odd-group\"><td></td><td></td>");
    for (j = 0; j < key.nstrings; ++j)
        printf("<td><hr></td>");
    printf("</tr>\n");

    printf("<tr class=\"even-group\"><td valign=\"top\">");
    printf("Listed %ld of %ld files.", (long)num_files, (long)total_files);
    printf("</td><td valign=\"top\" align=\"right\">Total:</td>\n");
    for (j = 0; j < key.nstrings; ++j)
    {
        double          x;

        x = sigma_x->data[j];
        printf("<td valign=\"top\" align=\"right\">%4.2f</td>\n", x);
    }
    printf("</tr>\n");

    printf("<tr class=\"even-group\">");
    printf("<td colspan=2 align=\"right\" valign=\"top\">Average:</td>\n");
    for (j = 0; j < key.nstrings; ++j)
    {
        double          x;

        x = sigma_x->data[j] / num_files;
        printf("<td valign=\"top\" align=\"right\">%4.2f</td>\n", x);
    }
    printf("</tr>\n");

    printf("<tr class=\"even-group\"><td colspan=2 align=\"right\" ");
    printf("valign=\"top\">Standard Deviation:</td>\n");
    for (j = 0; j < key.nstrings; ++j)
    {
        double          n;
        double          x;
        double          xx;
        double          mean;
        double          stddev;

        n = 1. / num_files;
        x = sigma_x->data[j];
        xx = sigma_x2->data[j];
        mean = x * n;
        stddev = sqrt(xx * n - mean * mean);
        printf("<td valign=\"top\" align=\"right\">%4.2f</td>\n", stddev);
    }
    printf("</tr>\n");

    printf("</table></div>\n");

    printf("<hr>\n");
    printf("<div class=\"report-cmd\">A similar report may be obtained\n");
    printf("from the command line, with one of\n");
    printf("<blockquote><samp>ael pf -p ");
    html_encode_string(project_name_get(pp));
    printf("<br>\n");
    printf("aer proj_files -p ");
    html_encode_string(project_name_get(pp));
    printf("</samp></blockquote></div>\n");

    printf("<hr>\n");
    printf("<p align=\"center\" class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(pp, "menu");
    printf("Project Menu</a> |\n");
    emit_project_href(pp, "changes");
    printf("Change List</a> |\n");
    emit_project_href(pp, "files");
    printf("File List</a>\n");
    printf("]</p>\n");

    html_footer(pp, cp);
}


// vim: set ts=8 sw=4 et :
