//
// aegis - project change supervisor
// Copyright (C) 2003-2009, 2011, 2012 Peter Miller
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

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/emit/brief_descri.h>
#include <libaegis/emit/edit_number.h>
#include <libaegis/file/event.h>
#include <libaegis/file/event/list.h>
#include <libaegis/http.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project.h>

#include <aeget/get/file/history.h>


void
get_file_history(change::pointer master_cp, string_ty *a_filename,
    string_list_ty *modifier)
{
    trace(("get_file_history()\n{\n"));
    nstring filename(a_filename);
    bool detailed = false;
    bool all = false;
    if (filename == ".")
        all = true;
    for (size_t k = 0; k < modifier->nstrings; ++k)
    {
        if (0 == strcasecmp(modifier->string[k]->str_text, "detailed"))
        {
            detailed = true;
            break;
        }
    }

    //
    // Reconstruct the project file history.
    //
    time_t when = master_cp->completion_timestamp();
    project *pp = master_cp->pp;
    project_file_roll_forward historian(pp, when, detailed);
    nstring_list key;
    historian.keys(key);

    html_header(pp, master_cp);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(", File History</title></head><body>\n");
    html_header_ps(pp, master_cp);
    printf("<h1 align=center>");
    emit_change(master_cp);
    printf(",<br>\nFile History</h1>\n");

    printf("<div class=\"information\">\n");
    printf("There is also a ");
    if (detailed)
    {
        emit_file_href(master_cp, filename, "file+history");
        printf("less");
    }
    else
    {
        emit_file_href(master_cp, filename, "file+history+detailed");
        printf("more");
    }
    printf("</a> detailed version of this listing available.\n");
    printf("<p>\n");
    printf("<table align=center>\n");
    printf("<tr class=\"even-group\"><th colspan=3>File Name</th>\n");
    printf("<th>Delta</th><th>Date and Time</th><th>Edit</th>\n");
    printf("<th>Description</th><th>&nbsp;</th></tr>\n");

    //
    // list the change's files' histories
    //
    size_t num_files = 0;
    for (size_t j = 0; j < key.size(); ++j)
    {
        nstring the_file_name = key[j];
        if (!all && filename != the_file_name)
            continue;
        if
        (
            !master_cp->bogus
        &&
            !master_cp->file_find(the_file_name, view_path_first)
        )
            continue;
        ++num_files;

        size_t num = 5;
        int usage_track = -1;
        int action_track = -1;
        string_ty *file_name_track = 0;

        file_event_list::pointer felp = historian.get(the_file_name);
        if (felp)
        {
            for (size_t k = 0; k < felp->size(); ++k)
            {
                file_event *fep = felp->get(k);
                assert(fep->get_src());
                if (!fep->get_src())
                    continue;

                if (!str_equal(file_name_track, fep->get_src()->file_name))
                {
                    file_name_track = fep->get_src()->file_name;

                    const char *html_class =
                        (((num / 3) & 1) ?  "even-group" : "odd-group");
                    ++num;
                    printf("<tr class=\"%s\">", html_class);

                    printf("<td colspan=8>\n");
                    emit_file_href(master_cp, file_name_track, "menu");
                    html_encode_string(file_name_track);
                    printf("</a>");
                    if (k > 0)
                        printf(" <i>(rename)</i>");
                    printf("</td></tr>\n");
                }

                const char *html_class =
                    (((num / 3) & 1) ?  "even-group" : "odd-group");
                ++num;
                printf("<tr class=\"%s\">", html_class);

                // filename column
                printf("<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>\n");

                // usage column
                printf("<td valign=\"top\">");
                if (usage_track != fep->get_src()->usage)
                {
                    printf("%s", file_usage_ename(fep->get_src()->usage));
                    usage_track = fep->get_src()->usage;
                }
                printf("</td>\n");

                // action column
                printf("<td valign=\"top\">");
                if (action_track != fep->get_src()->action)
                {
                    printf("%s", file_action_ename(fep->get_src()->action));
                    action_track = fep->get_src()->action;
                }
                printf("</td>\n");

                // delta column
                printf("<td valign=\"top\">");
                emit_change_href(fep->get_change(), "menu");
                nstring s = fep->get_change()->version_get();
                html_encode_string(s.get_ref());
                printf("</a></td>\n");

                // date and time column
                printf("<td valign=\"top\">");
                time_t when2 = fep->get_when();
                html_encode_charstar(ctime(&when2));
                printf("</td>\n");

                // change column
                printf("<td valign=\"top\" align=\"right\">");
                emit_file_href(fep->get_change(), fep->get_src()->file_name, 0);
                emit_edit_number(fep->get_change(), fep->get_src(), &historian);
                printf("</a></td>\n");

                // description column
                printf("<td valign=\"top\">\n");
                emit_change_brief_description(fep->get_change());
                printf("</td>\n");

                // download column
                printf("<td valign=\"top\">\n");
                if (fep->get_change()->download_files_accessable())
                {
                    emit_change_href(fep->get_change(), "download");
                    printf("Download</a>\n");
                }
                printf("</td></tr>\n");
            }
        }

        //
        // Now output details of this change, as the "end"
        // of the history.
        //
        if (!master_cp->bogus && !master_cp->is_completed())
        {
            fstate_src_ty   *src_data;

            src_data = master_cp->file_find(the_file_name, view_path_first);
            if (src_data)
            {
                const char      *html_class;

                html_class = (((num / 3) & 1) ?  "even-group" : "odd-group");
                printf("<tr class=\"%s\">", html_class);

                // filename column
                printf("<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>\n");

                // usage column
                printf("<td valign=\"top\">\n");
                if (usage_track != src_data->usage)
                    printf("%s", file_usage_ename(src_data->usage));
                printf("</td>\n");

                // action column
                printf("<td valign=\"top\">\n");
                if (action_track != src_data->action)
                    printf("%s", file_action_ename(src_data->action));
                printf("</td>\n");

                // delta column
                printf("<td valign=\"top\">\n");
                emit_change_href(master_cp, "menu");
                html_encode_string(master_cp->version_get());
                printf("</a></td>\n");

                // date and time column
                printf("<td></td>\n");

                // change column
                printf("<td valign=\"top\" align=\"right\">\n");
                emit_change_href(master_cp, "menu");
                printf("%ld", magic_zero_decode(master_cp->number));
                printf("</a></td>\n");

                // description column
                printf("<td valign=\"top\">\n");
                emit_change_brief_description(master_cp);
                printf("</td>\n");

                // download column
                printf("<td valign=\"top\">");
                if (master_cp->download_files_accessable())
                {
                    emit_change_href(master_cp, "download");
                    printf("Download</a>\n");
                }
                printf("</td></tr>\n");
            }
        }
    }
    printf("<tr class=\"even-group\"><td colspan=8>");
    printf("Listed %ld file%s.", (long)num_files, (num_files == 1 ? "" : "s"));
    printf("</td></tr>\n");

    printf("</table>\n");
    printf("<p>\n");
    printf("There is also a ");
    if (detailed)
    {
        emit_file_href(master_cp, filename, "file+history");
        printf("less");
    }
    else
    {
        emit_file_href(master_cp, filename, "file+history+detailed");
        printf("more");
    }
    printf("</a> detailed version of this listing available.\n");
    printf("</div>\n");
    html_footer(pp, master_cp);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
