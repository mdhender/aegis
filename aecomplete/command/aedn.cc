//
//      aegis - project change supervisor
//      Copyright (C) 2002-2008, 2011, 2012 Peter Miller
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

#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <aecomplete/command/aedn.h>
#include <aecomplete/command/generic.h>
#include <aecomplete/command/private.h>
#include <aecomplete/complete/change/number.h>
#include <aecomplete/complete/nil.h>
#include <aecomplete/complete/project/delta.h>
#include <aecomplete/complete/project/name.h>
#include <libaegis/project.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>


static void
destructor(command_ty *)
{
}


static complete_ty *
completion_get(command_ty *)
{
    string_ty       *project_name;
    complete_ty     *result;
    int             incomplete_change_number;
    int             incomplete_delta_number;
    project      *pp;

    arglex2_retable(0);
    arglex();
    project_name = 0;
    incomplete_change_number = 0;
    incomplete_delta_number = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            result = generic_argument_complete();
            if (result)
                return result;
            continue;

        case arglex_token_string:
            // what to name it, ignore
            break;

        case arglex_token_string_incomplete:
            // incomplete name
            break;

        case arglex_token_delta:
            switch (arglex())
            {
            default:
                continue;

            case arglex_token_number:
                break;

            case arglex_token_number_incomplete:
            case arglex_token_string_incomplete:
                incomplete_delta_number = 1;
                break;
            }
            break;

        case arglex_token_number:
            // delta number, ignore
            break;

        case arglex_token_number_incomplete:
            incomplete_delta_number = 1;
            break;

        case arglex_token_project:
            switch (arglex())
            {
            default:
                continue;

            case arglex_token_string:
                project_name = str_from_c(arglex_value.alv_string);
                break;

            case arglex_token_string_incomplete:
            case arglex_token_number_incomplete:
                return complete_project_name();
            }
            break;

        case arglex_token_delta_date:
            switch (arglex())
            {
            default:
                continue;

            case arglex_token_string:
            case arglex_token_number:
                break;

            case arglex_token_string_incomplete:
            case arglex_token_number_incomplete:
                // maybe "complete_project_delta_date"
                return complete_nil();
                break;
            }
            break;

        case arglex_token_delta_from_change:
            switch (arglex())
            {
            default:
                continue;

            case arglex_token_number:
                break;

            case arglex_token_number_incomplete:
            case arglex_token_string_incomplete:
                incomplete_change_number = 1;
                break;
            }
            break;
        }
        arglex();
    }

    //
    // Work out which project to use.
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
        project_name = str_copy(n.get_ref());
    }
    pp = project_alloc(project_name);
    pp->bind_existing();

    //
    // If we need to complete a change number, we have the project now.
    //
    if (incomplete_change_number)
        return complete_change_number(pp, 1 << cstate_state_completed);

    //
    // If we need to complete a delta number, we have the project now,
    // and the branch to use.
    //
    if (incomplete_delta_number)
        return complete_project_delta(pp);

    //
    // Sorry, can't help you with making up the name.
    //
    return complete_nil();
}


static command_vtbl_ty vtbl =
{
    destructor,
    completion_get,
    sizeof(command_ty),
    "aedn",
};


command_ty *
command_aedn()
{
    return command_new(&vtbl);
}


// vim: set ts=8 sw=4 et :
