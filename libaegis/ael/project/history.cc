//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/project/history.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/cstate.h>
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/user.h>


void
list_project_history(change_identifier &cid, string_list_ty *)
{
    output::pointer name_col;
    output::pointer delta_col;
    output::pointer date_col;
    output::pointer change_col;
    output::pointer description_col;
    size_t          j, k;
    string_ty       *line1;
    int             left;

    trace(("list_project_history()\n{\n"));
    if (cid.set())
        list_change_inappropriate();

    //
    // create the columns
    //
    col::pointer colp = col::open((string_ty *)0);
    line1 = str_format("Project \"%s\"", cid.get_pp()->name_get()->str_text);
    colp->title(line1->str_text, "History");
    str_free(line1);

    // the delta name column is the whole page wide
    name_col = colp->create(0, 0, (const char *)0);

    left = 0;
    delta_col = colp->create(left, left + CHANGE_WIDTH, "Delta\n-------");
    left += CHANGE_WIDTH + 1;

    if (!option_terse_get())
    {
        date_col =
            colp->create
            (
                left,
                left + WHEN_WIDTH,
                "Date and Time\n---------------"
            );
        left += WHEN_WIDTH + 1;

        change_col =
            colp->create(left, left + CHANGE_WIDTH, "Change\n-------");
        left += CHANGE_WIDTH + 1;

        description_col =
            colp->create(left, 0, "Description\n-------------");
    }

    //
    // list the project's successful integrations
    //
    for (j = 0; ; ++j)
    {
        long            cn;
        long            dn;
        string_list_ty  name;

        if (!project_history_nth(cid.get_pp(), j, &cn, &dn, &name))
            break;
        if (!option_terse_get() && name.nstrings)
        {
            colp->need(4);
            name_col->fprintf("Name%s: ", (name.nstrings == 1 ? "" : "s"));
            for (k = 0; k < name.nstrings; ++k)
            {
                if (k)
                    name_col->fputs(", ");
                name_col->fprintf("\"%s\"", name.string[k]->str_text);
            }

            //
            // If we don't eoln here, and there are lots
            // of names, then they get intermingled with
            // the date and description lines, and it
            // looks weird.
            //
            colp->eoln();
        }
        delta_col->fprintf("%4ld", dn);
        if (!option_terse_get())
        {
            change::pointer cp = change_alloc(cid.get_pp(), cn);
            change_bind_existing(cp);
            cstate_ty *cstate_data = cp->cstate_get();
            time_t t =
                cstate_data->history->list
                [
                        cstate_data->history->length - 1
                ]->when;
            date_col->fputs(ctime(&t));
            change_col->fprintf("%4ld", magic_zero_decode(cn));
            assert(cstate_data->brief_description);
            description_col->fputs(cstate_data->brief_description);
            change_free(cp);
        }
        colp->eoln();
    }
    trace(("}\n"));
}
