//
//      aegis - project change supervisor
//      Copyright (C) 2004-2009, 2011, 2012 Peter Miller
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

#include <common/libdir.h>
#include <common/now.h>
#include <common/nstring.h>
#include <common/str_list.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/emit/brief_descri.h>
#include <libaegis/emit/edit_number.h>
#include <libaegis/file/event.h>
#include <libaegis/file/event/list.h>
#include <libaegis/http.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/zero.h>

#include <aeget/get/command.h>
#include <aeget/get/file/diff.h>

#define XRANGE 3
#define YRANGE 7


static bool
same_change(change::pointer cp1, change::pointer cp2)
{
    if (cp1 == cp2)
        return true;
    if (cp1->number != cp2->number)
        return false;
    if (cp1->pp == cp2->pp)
        return true;
    return str_equal(project_name_get(cp1->pp), project_name_get(cp2->pp));
}


void
get_file_diff(change::pointer master_cp, string_ty *filename,
    string_list_ty *modifier_p)
{
    string_list_ty &modifier = *modifier_p;
    bool context = false;
    bool unified = false;
    bool detailed = false;
    nstring rhs;
    for (size_t j = 0; j < modifier.size(); ++j)
    {
        string_ty *s = modifier[j];
        if (0 == strcasecmp(s->str_text, "detailed"))
        {
            detailed = true;
        }
        if (0 == strcasecmp(s->str_text, "context"))
        {
            context = true;
        }
        if (0 == strcasecmp(s->str_text, "unified"))
        {
            unified = true;
        }
        if (0 == strncasecmp(s->str_text, "rhs=", 4))
        {
            rhs = s->str_text + 4;
        }
    }
    if (!rhs.empty())
    {
        //
        // Form the command to be executed.
        //
        nstring qp = project_name_get(master_cp->pp).quote_shell();
        string_ty *qf = str_quote_shell(filename);
        string_ty *command =
            str_format
            (
                "%s/aediff %s -p %s -c %ld -c %s %s",
                configured_bindir(),
                (unified ? "-unified" : (context ? "-context" : "")),
                qp.c_str(),
                magic_zero_decode(master_cp->number),
                rhs.quote_shell().c_str(),
                qf->str_text
            );
        str_free(qf);

        //
        // Print the content type as the response header, otherwise
        // things get messy.
        //
        printf("Content-Type: text/plain\n\n");
        fflush(stdout);

        //
        // Run the command.
        // This will exec() and does not return.
        //
        get_command(command->str_text);
        // NOTREACHED
        return;
    }

    //
    // Reconstruct the project file history.
    //
    time_t when = now();
    project *pp = master_cp->pp;
    project_file_roll_forward historian(pp, when, detailed);

    //
    // Emit page title.
    //
    html_header(pp, master_cp);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(", File History</title></head><body>\n");

    //
    // Emit page header.
    //
    html_header_ps(pp, master_cp);
    printf("<h1 align=center>");
    emit_change(master_cp);
    printf(",<br>\nFile &ldquo;");
    emit_file_href(master_cp, filename, "menu");
    html_encode_string(filename);
    printf("</a>&rdquo;,<br>Difference Table</h1>\n");

    file_event_list::pointer felp = historian.get(filename);
    assert(!felp || !felp->empty());
    if (!felp)
    {
        printf("This does not appear to be a source file;\n");
        printf("no details available.\n");

        //
        // Emit page footer, then run away screaming.
        //
        html_footer(pp, master_cp);
        return;
    }
    if (felp->size() == 1)
    {
        printf("There is only one version of this file\n(");
        file_event *fep = felp->get(0);
        assert(fep->get_src());
        if (fep->get_src())
            emit_file_href(fep->get_change(), fep->get_src()->file_name, 0);
        nstring s = fep->get_change()->version_get();
        html_encode_charstar(s.c_str());
        if (fep->get_src())
            printf("</a>");
        printf(")\n so there is no difference table available.\n");

        //
        // Emit page footer, then run away screaming.
        //
        html_footer(pp, master_cp);
        return;
    }

    //
    // Find the point in the list of things which happened to this file
    // that corresponds to the master change.
    //
    size_t idx = 0;
    for (; idx < felp->size(); ++idx)
    {
        file_event *fep = felp->get(idx);
        if (same_change(fep->get_change(), master_cp))
            break;
    }

    size_t x_last = idx + (XRANGE + 1);
    if (x_last > felp->size())
        x_last = felp->size();
    size_t x_first = x_last - (2 * XRANGE + 1);
    if (x_last < (2 * XRANGE + 1))
        x_first = 0;

    size_t y_last = idx + (YRANGE + 1);
    if (y_last > felp->size())
        y_last = felp->size();
    size_t y_first = y_last - (2 * YRANGE + 1);
    if (y_last < (2 * YRANGE + 1))
        y_first = 0;

    printf("<div class=\"information\">\n");
    printf("There is also a ");
    if (detailed)
    {
        emit_file_href(master_cp, filename, "diff");
        printf("less");
    }
    else
    {
        emit_file_href(master_cp, filename, "diff+detailed");
        printf("more");
    }
    printf("</a> detailed version of this listing available.\n");
    printf("<p>\n");

    //
    // Emit table headings.
    //
    printf("The version numbers along the top and left are\n");
    printf("linked to the change menu of the relevant change\n");
    printf("set.  The edit number in the <strong>Edit</strong>\n");
    printf("column are linked to historical versions of the\n");
    printf("file.  Each cell has links to ordinary file diffs,\n");
    printf("context diff (-c) and unified diff (-u).  Cells marked\n");
    printf("&ldquo;(rev)&rdquo; are backwards diffs, all others\n");
    printf("are forwards diffs.\n");
    printf("<p><table align=center>\n");
    printf("<tr class=\"even-group\"><td>&nbsp;</td>\n");
    for (size_t x = x_first; x < x_last; ++x)
    {
        printf("<td align=center>");
        file_event *fep = felp->get(x);
        bool strong = same_change(fep->get_change(), master_cp);
        nstring s = fep->get_change()->version_get();
        if (strong)
            printf("<strong>");
        emit_change_href(fep->get_change(), "menu");
        html_encode_charstar(s.c_str());
        printf("</a>");
        if (strong)
            printf("</strong>");
        printf("</td>\n");
    }
    printf("<td align=center><b>Edit</b></td>");
    printf("<td align=center><b>Description</b></td></tr>\n");

    //
    // Emit the grid of differences.
    //
    int rownum = 0;
    for (size_t y = y_first; y < y_last; ++y)
    {
        //
        // Emit version.
        //
        const char *html_class = (((rownum++ / 3) & 1) ?  "even" : "odd");
        printf("<tr class=\"%s-group\">", html_class);
        printf("<td valign=top>");
        file_event *y_fep = felp->get(y);
        bool y_strong = same_change(y_fep->get_change(), master_cp);
        nstring s = y_fep->get_change()->version_get();
        if (y_strong)
            printf("<strong>");
        emit_change_href(y_fep->get_change(), "menu");
        html_encode_charstar(s.c_str());
        printf("</a>");
        if (y_strong)
            printf("</strong>");
        printf("</td>\n");

        //
        // Emit diff cells.
        //
        for (size_t x = x_first; x < x_last; ++x)
        {
            printf("<td valign=top align=center>");
            file_event *x_fep = felp->get(x);
            bool x_strong = same_change(x_fep->get_change(), master_cp);
            if (x_strong || y_strong)
                printf("<strong>");
            if (x_fep->get_change() == y_fep->get_change())
            {
                printf("*");
            }
            else
            {
                nstring rs = x_fep->get_change()->version_get();
                nstring target = nstring::format("diff+rhs=%s", rs.c_str());
                emit_file_href(y_fep->get_change(), filename, target.c_str());
                printf("diff</a> / ");
                target = nstring::format("diff+rhs=%s+context", rs.c_str());
                emit_file_href(y_fep->get_change(), filename, target.c_str());
                printf("-c</a> / ");
                target = nstring::format("diff+rhs=%s+unified", rs.c_str());
                emit_file_href(y_fep->get_change(), filename, target.c_str());
                printf("-u</a>");
            }
            if (x_strong || y_strong)
                printf("</strong>");
            if (x < y)
                printf(" (rev)");
            printf("</td>\n");
        }

        //
        // Emit the edit number.
        //
        printf("<td valign=top>");
        assert(y_fep->get_src());
        if (y_fep->get_src())
            emit_file_href(y_fep->get_change(), y_fep->get_src()->file_name, 0);
        emit_edit_number(y_fep->get_change(), y_fep->get_src(), &historian);
        if (y_fep->get_src())
            printf("</a>");
        printf("</td>\n");

        //
        // Emit the brief description.
        //
        printf("<td valign=top>");
        emit_change_brief_description(y_fep->get_change());
        printf("</td></tr>\n");
    }

    //
    // There is no sumamry row.
    //
    printf("</table>\n");
    printf("<p>\n");
    printf("There is also a ");
    if (detailed)
    {
        emit_file_href(master_cp, filename, "diff");
        printf("less");
    }
    else
    {
        emit_file_href(master_cp, filename, "diff+detailed");
        printf("more");
    }
    printf("</a> detailed version of this listing available.\n");
    printf("</div>\n");

    //
    // Emit page footer.
    //
    html_footer(pp, master_cp);
}


// vim: set ts=8 sw=4 et :
