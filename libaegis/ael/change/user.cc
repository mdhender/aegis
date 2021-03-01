//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2008, 2011, 2012 Peter Miller
//      Copyright (C) 2007, 2008 Walter Franzini
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
//      <http://www.gnu.org/licenses/>.
//

#include <common/error.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/change/user.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/project/inappropriat.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


void
list_user_changes(change_identifier &cid, string_list_ty *args)
{
    string_ty       *s;
    string_list_ty  name;

    trace(("list_user_changes()\n{\n"));
    if (cid.project_set())
        list_project_inappropriate();
    if (cid.set())
        list_change_inappropriate();

    //
    // get the list of projects
    //
    project_list_get(&name);
    if (!name.nstrings)
    {
        trace(("}\n"));
        return;
    }

    user_ty::pointer up;
    if (!args || !args->nstrings)
    {
        //
        // No user name is provided, use the current user.
        //
        up = user_ty::create();
    }
    else
    {
        //
        // Use the user name supplied by the caller.
        //
        up = user_ty::create(nstring(args->string[0]));
    }

    //
    // open listing
    //
    col::pointer colp = col::open((string_ty *)0);
    s =
        str_format
        (
            "Owned by %s <%s>",
            up->full_name().c_str(),
            up->name().c_str()
        );
    colp->title("List of Changes", s->str_text);
    str_free(s);

    //
    // create the columns
    //
    int left = 0;
    output::pointer project_col =
        colp->create(left, left + PROJECT_WIDTH, "Project\n----------");
    left += PROJECT_WIDTH + 1;
    output::pointer change_col =
        colp->create(left, left + CHANGE_WIDTH, "Change\n------");
    left += CHANGE_WIDTH + 1;
    output::pointer state_col =
        colp->create(left, left + STATE_WIDTH, "State\n----------");
    left += STATE_WIDTH + 1;
    output::pointer description_col =
        colp->create(left, 0, "Description\n-------------");

    //
    // for each project, see if the current user
    // is working on any of them.
    //
    for (size_t j = 0; j < name.nstrings; ++j)
    {
        //
        // locate the project,
        // and make sure we are allowed to look at it
        //
        project *pp = project_alloc(name.string[j]);
        pp->bind_existing();
        int err = project_is_readable(pp);
        if (err != 0)
        {
            project_free(pp);
            continue;
        }

        //
        // for each change within this project the user
        // is working on emit a line of information
        //
        for (long n = 0;; ++n)
        {
            long change_number = 0;
            if (!up->own_nth(pp, n, change_number))
                break;

            //
            // locate change data
            //
            change::pointer cp = change_alloc(pp, change_number);
            change_bind_existing(cp);

            //
            // emit the info
            //
            project_col->fputs(project_name_get(pp).c_str());
            change_col->fprintf("%4ld", magic_zero_decode(change_number));
            cstate_ty *cstate_data = cp->cstate_get();
            state_col->fputs(cstate_state_ename(cstate_data->state));
            if (cstate_data->brief_description)
            {
                description_col->fputs
                (
                    cstate_data->brief_description->str_text
                );
            }
            colp->eoln();

            //
            // release change and project
            //
            change_free(cp);
        }

        //
        // free project
        //
        project_free(pp);
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
