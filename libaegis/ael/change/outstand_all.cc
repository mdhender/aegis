//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2008, 2011, 2012 Peter Miller
//      Copyright (C) 2008 Walter Franzini
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

#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/change/outstand_all.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/project/inappropriat.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <common/str_list.h>
#include <common/trace.h>


void
list_outstanding_changes_all(change_identifier &cid, string_list_ty *)
{
    trace(("list_outstanding_changes_all()\n{\n"));
    if (cid.project_set())
        list_project_inappropriate();
    if (cid.set())
        list_change_inappropriate();

    //
    // list the projects
    //
    string_list_ty name;
    project_list_get(&name);

    //
    // create the columns
    //
    col::pointer colp = col::open((string_ty *)0);
    colp->title("List of Outstanding Changes", "for all projects");

    int left = 0;
    output::pointer project_col =
        colp->create(left, left + PROJECT_WIDTH, "Project\n---------");
    left += PROJECT_WIDTH + 1;

    output::pointer number_col =
        colp->create(left, left + CHANGE_WIDTH, "Change\n------");
    left += CHANGE_WIDTH + 1;

    output::pointer state_col;
    output::pointer description_col;
    if (!option_terse_get())
    {
        state_col =
            colp->create(left, left + STATE_WIDTH, "State\n-------");
        left += STATE_WIDTH + 1;

        description_col =
            colp->create(left, 0, "Description\n-------------");
    }

    //
    // scan each project
    //
    for (size_t j = 0; j < name.nstrings; ++j)
    {
        project *pp = project_alloc(name.string[j]);
        pp->bind_existing();

        //
        // make sure we have permission
        //
        int err = project_is_readable(pp);
        if (err != 0)
        {
            project_free(pp);
            continue;
        }

        //
        // list the project's changes
        //
        for (size_t k = 0; ; ++k)
        {
            //
            // make sure the change is not completed
            //
            long change_number = 0;
            if (!project_change_nth(pp, k, &change_number))
                break;
            change::pointer cp = change_alloc(pp, change_number);
            change_bind_existing(cp);
            cstate_ty *cstate_data = cp->cstate_get();
            if (cstate_data->state == cstate_state_completed)
            {
                change_free(cp);
                continue;
            }

            //
            // print the details
            //
            project_col->fputs(project_name_get(pp));
            number_col->fprintf("%4ld", magic_zero_decode(change_number));
            if (state_col)
            {
                state_col->fputs(cstate_state_ename(cstate_data->state));
                if
                (
                    option_verbose_get()
                &&
                    cstate_data->state == cstate_state_being_developed
                )
                {
                    state_col->end_of_line();
                    state_col->fputs(cp->developer_name());
                }
                if
                (
                    option_verbose_get()
                &&
                    cstate_data->state == cstate_state_being_integrated
                )
                {
                    state_col->end_of_line();
                    state_col->fputs(cp->integrator_name());
                }
            }
            if (description_col && cstate_data->brief_description)
            {
                description_col->fputs(cstate_data->brief_description);
            }
            colp->eoln();

            //
            // At some point, will need to recurse
            // if it is a branch and not a leaf.
            //
            change_free(cp);
        }
        project_free(pp);
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
