//
// aegis - project change supervisor
// Copyright (C) 1999, 2001, 2003-2008, 2011, 2012 Peter Miller
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

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/history.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/aer/func/now.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/user.h>


void
list_change_history(change_identifier &cid, string_list_ty *)
{
    trace(("list_change_history()\n{\n"));
    cstate_ty *cstate_data = cid.get_cp()->cstate_get();

    //
    // create the columns
    //
    col::pointer colp = col::open((string_ty *)0);
    string_ty *line1 =
        str_format
        (
            "Project \"%s\"  Change %ld",
            project_name_get(cid.get_pp()).c_str(),
            cid.get_change_number()
        );
    colp->title(line1->str_text, "History");
    str_free(line1);

    int left = 0;
    output::pointer what_col =
        colp->create(left, left + WHAT_WIDTH, "What\n------");
    left += WHAT_WIDTH + 1;

    output::pointer when_col =
        colp->create(left, left + WHEN_WIDTH, "When\n------");
    left += WHEN_WIDTH + 1;

    output::pointer who_col =
        colp->create(left, left + WHO_WIDTH, "Who\n-----");
    left += WHO_WIDTH + 1;

    output::pointer why_col = colp->create(left, 0, "Comment\n---------");

    //
    // list the history
    //
    for (size_t j = 0; j < cstate_data->history->length; ++j)
    {
        cstate_history_ty *history_data = cstate_data->history->list[j];
        what_col->fputs(cstate_history_what_ename(history_data->what));
        time_t t = history_data->when;
        when_col->fputs(ctime(&t));
        who_col->fputs(history_data->who->str_text);
        if (history_data->why)
            why_col->fputs(history_data->why->str_text);
        if (history_data->what != cstate_history_what_integrate_pass)
        {
            time_t finish = 0;
            if (j + 1 < cstate_data->history->length)
                finish = cstate_data->history->list[j + 1]->when;
            else
                time(&finish);
            if (finish - t >= ELAPSED_TIME_THRESHOLD)
            {
                why_col->end_of_line();
                why_col->fprintf
                (
                    "Elapsed time: %5.3f days.\n",
                    working_days(t, finish)
                );
            }
        }
        colp->eoln();
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
