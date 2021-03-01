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
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <common/nstring.h>
#include <common/str_list.h>
#include <common/symtab.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/emit/project.h>
#include <libaegis/http.h>
#include <libaegis/os.h>
#include <libaegis/project.h>

#include <aeget/get/file/cause_densit.h>


static const char *
color_by_change_cause(change_cause_ty x)
{
    switch (x)
    {
    case change_cause_chain:
        return "255,0,0";

    case change_cause_internal_bug:
        return "255,255,0";

    case change_cause_external_bug:
        return "192,192,0";

    case change_cause_internal_enhancement:
        return "0,255,0";

    case change_cause_external_enhancement:
        return "0,192,0";

    case change_cause_internal_improvement:
        return "0,255,255";

    case change_cause_external_improvement:
        return "0,192,192";
    }
    return "64,64,64";
}


static int
height_by_file_action(file_action_ty x)
{
    switch (x)
    {
    case file_action_create:
        return HISTOGRAM_HEIGHT;

    case file_action_modify:
        return HISTOGRAM_HEIGHT + (HISTOGRAM_HEIGHT / 3);

    case file_action_remove:
        return HISTOGRAM_HEIGHT - (HISTOGRAM_HEIGHT / 3);

    case file_action_insulate:
        // should never be seen in a completed change
        return HISTOGRAM_HEIGHT + (HISTOGRAM_HEIGHT / 4);

    case file_action_transparent:
        return HISTOGRAM_HEIGHT - (HISTOGRAM_HEIGHT / 2);
    }
    return HISTOGRAM_HEIGHT;
}


static void
reaper(void *p)
{
    delete (long *)p;
}

static void
incr(symtab_ty &st, const nstring &key)
{
    long *p = (long *)st.query(key.get_ref());
    if (!p)
    {
        p = new long(0);
        st.assign(key.get_ref(), p);
        st.set_reap(reaper);
    }
    ++*p;
}


void
get_file_cause_density(change::pointer master_cp, string_ty *filename,
    string_list_ty *)
{
    if (!filename || !strcmp(filename->str_text, "."))
        filename = str_from_c("");

    //
    // Emit the page title.
    //
    project *pp = master_cp->pp;
    html_header(pp, master_cp);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(", Change Density by File</title></head><body>\n");

    //
    // Emit the page heading.
    //
    html_header_ps(pp, master_cp);
    printf("<h1 align=center>");
    emit_project(pp);
    printf(",<br>\nChange Density by File");
    if (filename->str_length)
    {
        printf(",<br>\n");
        html_encode_string(filename);
    }
    printf("</h1>\n");

    //
    // Create a symbol table, indexed by cause by filename,
    // each row is a list of changes involving that file.
    //
    symtab_ty directory;
    symtab_ty file_cause_action[change_cause_max][file_action_max];
    symtab_ty file_total;
    long cause_total[change_cause_max];
    for (int j1 = 0; j1 < change_cause_max; ++j1)
        cause_total[j1] = 0;
    long action_total[file_action_max];
    for (int j2 = 0; j2 < file_action_max; ++j2)
        action_total[j2] = 0;

    //
    // Go through the project's changes,
    // remembering the relevant ones.
    //
    change::pointer pcp = pp->change_get();
    cstate_ty *proj_cstate_data = pcp->cstate_get();
    for (size_t j = 0; j < proj_cstate_data->branch->history->length; ++j)
    {
        cstate_branch_history_ty *hp =
            proj_cstate_data->branch->history->list[j];
        change::pointer cp = change_alloc(pp, hp->change_number);
        change_bind_existing(cp);
        assert(cp->is_completed());
        cstate_ty *cstate_data = cp->cstate_get();

        for (size_t file_num = 0; ; ++file_num)
        {
            fstate_src_ty *src = change_file_nth(cp, file_num, view_path_first);
            if (!src)
                break;

            string_ty *rhs =
                (
                    filename->str_length
                ?
                    os_below_dir(filename, src->file_name)
                :
                    str_copy(src->file_name)
                );
            if (!rhs)
                continue;
            nstring key(rhs);
            rhs = 0;

            //
            // Track files by change cause.
            //
            const char *ep = strchr(key.c_str(), '/');
            if (ep)
            {
                key = nstring(key.c_str(), ep - key.c_str());
                incr(directory, key);
            }
            incr(file_total, key);
            incr(file_cause_action[cstate_data->cause][src->action], key);
            cause_total[cstate_data->cause]++;
            action_total[src->action]++;
        }
        change_free(cp);
    }
    nstring prefix;
    if (filename->str_length)
        prefix = nstring::format("%s/", filename->str_text);

    //
    // Figure out the scale for the histogram.
    //
    string_list_ty keys;
    file_total.keys(&keys);
    keys.sort();
    long imax = 1;
    for (size_t j3 = 0; j3 < keys.size(); ++j3)
    {
        string_ty *the_file_name = keys[j3];
        long *p = (long *)file_total.query(the_file_name);
        assert(p);
        long n = (p ? *p : 0);
        if (imax < n)
            imax = n;
    }
    double dmax = 400. / imax;

    //
    // Emit the columns headings.
    //
    printf("<div class=\"information\"><table align=\"center\">\n");
    printf("<tr class=\"even-group\"><th>Filename</th><th>Count</th>");
    printf("<th>&nbsp;</th></tr>\n");

    //
    // For each file in the symbol table,
    // list the changes working on it at the moment.
    //
    long num_files = 0;
    long rownum = 0;
    for (size_t j4 = 0; j4 < keys.size(); ++j4)
    {
        //
        // Emit the file name.
        //
        string_ty *the_file_name = keys[j4];
        const char *html_class = (((rownum++ / 3) & 1) ?  "even" : "odd");
        printf("<tr class=\"%s-group\">", html_class);
        printf("<td>");
        bool is_a_dir = (0 != directory.query(the_file_name));
        nstring full_file_name =
            nstring::format("%s%s", prefix.c_str(), the_file_name->str_text);
        emit_file_href
        (
            master_cp,
            full_file_name.get_ref(),
            (is_a_dir ? "file+cause-density" : "file+menu")
        );
        html_encode_string(the_file_name);
        printf("</a>");
        if (is_a_dir)
            printf("/");
        else
            ++num_files;
        printf("</td>\n");

        //
        // Emit the count for this file.
        //
        long *p = (long *)file_total.query(the_file_name);
        assert(p);
        long n = (p ? *p : 0);
        printf("<td align=right>%ld</td>\n", n);

        //
        // Emit the histogram for this file.
        //
        printf("<td>");
        for (int ax = 0; ax < file_action_max; ++ax)
        {
            int height = height_by_file_action((file_action_ty)ax);
            for (int cx = 0; cx < change_cause_max; ++cx)
            {
                p = (long *)file_cause_action[cx][ax].query(the_file_name);
                n = (p ? *p : 0);
                if (n > 0)
                {
                    int width = (int)(0.5 + n * dmax);
                    const char *color =
                        color_by_change_cause((change_cause_ty)cx);
                    emit_rect_image_rgb(width, height, color, 0);
                }
            }
        }
        printf("</td></tr>\n");
    }

    //
    // Emit the summary row.
    //
    printf("<tr class=\"even-group\"><td colspan=8>Listed ");
    printf("%ld file%s.</td></tr>\n", num_files, (num_files == 1 ? "" : "s"));
    printf("</table></div>\n");

    //
    // Emit the legend.
    //
    printf("<hr>");
    printf("<div class=\"legend\">");
    printf("<h2 align=center>Legend</h2>");
    printf("<table align=center>\n");
    printf("<th>");
    for (int bx = 0; bx < file_action_max; ++bx)
    {
        if (action_total[bx] == 0)
            continue;
        printf("<th>%s</th>", file_action_ename((file_action_ty)bx));
    }
    printf("<th>&nbsp;</th><th>&nbsp;</th></tr>\n");
    for (int cx = 0; cx < change_cause_max; ++cx)
    {
        if (cause_total[cx] == 0)
            continue;
        printf("<tr>");
        printf("<td>%s</td>\n", change_cause_ename((change_cause_ty)cx));
        int width = 30;
        const char *color = color_by_change_cause((change_cause_ty)cx);
        for (int dx = 0; dx < file_action_max; ++dx)
        {
            if (action_total[dx] == 0)
                continue;
            int height = height_by_file_action((file_action_ty)dx);
            printf("<td align=center>");
            emit_rect_image_rgb(width, height, color);
            printf("</td>\n");
        }
        printf("<td align=right>%ld</td></tr>\n", cause_total[cx]);
    }
    printf("<tr><td>&nbsp;</td>");
    for (int fx = 0; fx < file_action_max; ++fx)
    {
        if (action_total[fx] == 0)
            continue;
        printf("<td align=center>%ld</td>\n", action_total[fx]);
    }
    printf("<td>&nbsp;</td></tr>\n");
    printf("</table></div>\n");

    //
    // Emit the navigation bar.
    //
    printf("<hr>\n");
    printf("<p align=\"center\" class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(pp, "menu");
    printf("Project Menu</a> |\n");
    emit_project_href(pp, "changes");
    printf("Change List</a>\n");
    printf("]</p>\n");

    //
    // Emit the page footer.
    //
    html_footer(pp, master_cp);
}


// vim: set ts=8 sw=4 et :
