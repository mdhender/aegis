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
#include <aecomplete/command/aecp.h>
#include <aecomplete/command/generic.h>
#include <aecomplete/command/private.h>
#include <aecomplete/complete/nil.h>
#include <aecomplete/complete/change/number.h>
#include <aecomplete/complete/project/branch.h>
#include <aecomplete/complete/project/delta.h>
#include <aecomplete/complete/project/file.h>
#include <aecomplete/complete/project/name.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>


static void
destructor(command_ty *)
{
}


static void
do_nothing(void)
{
}


static complete_ty *
completion_get(command_ty *)
{
    string_ty       *project_name;
    int             overwriting;
    int             baserel;
    complete_ty     *result;
    int             incomplete_change_number;
    int             incomplete_delta;
    int             incomplete_branch;
    int             change_number;
    const char      *branch;
    project      *pp;
    user_ty::pointer up;
    change::pointer cp;
    int             independent;

    arglex2_retable(0);
    arglex();
    project_name = 0;
    overwriting = 0;
    incomplete_change_number = 0;
    incomplete_delta = 0;
    incomplete_branch = 0;
    change_number = 0;
    branch = 0;
    independent = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            result = generic_argument_complete();
            if (result)
                return result;
            continue;

        case arglex_token_overwriting:
            overwriting = 1;
            break;

        case arglex_token_directory:
        case arglex_token_file:
            // noise word, ignore
            break;

        case arglex_token_string:
            // whole file name, ignore
            break;

        case arglex_token_string_incomplete:
            // incomplete file name
            break;

        case arglex_token_change:
            switch (arglex())
            {
            default:
                continue;

            case arglex_token_number:
                if (arglex_value.alv_number == 0)
                    change_number = MAGIC_ZERO;
                else if (arglex_value.alv_number > 0)
                    change_number = arglex_value.alv_number;
                break;

            case arglex_token_number_incomplete:
            case arglex_token_string_incomplete:
                incomplete_change_number = 1;
                break;
            }
            break;

        case arglex_token_number:
            if (arglex_value.alv_number == 0)
                change_number = MAGIC_ZERO;
            else if (arglex_value.alv_number > 0)
                change_number = arglex_value.alv_number;
            break;

        case arglex_token_number_incomplete:
            incomplete_change_number = 1;
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

        case arglex_token_independent:
            independent = 1;
            break;

        case arglex_token_delta:
            switch (arglex())
            {
            default:
                continue;

            case arglex_token_string:
            case arglex_token_number:
                break;

            case arglex_token_string_incomplete:
            case arglex_token_number_incomplete:
                incomplete_delta = 1;
                break;
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
                incomplete_change_number = 2;
                break;
            }
            break;

        case arglex_token_branch:
            switch (arglex())
            {
            default:
                continue;

            case arglex_token_number:
            case arglex_token_string:
                branch = arglex_value.alv_string;
                break;

            case arglex_token_stdio:
                branch = "";
                break;

            case arglex_token_number_incomplete:
            case arglex_token_string_incomplete:
                incomplete_branch = 1;
                break;
            }
            break;

        case arglex_token_trunk:
            branch = "";
            break;

        case arglex_token_base_relative:
        case arglex_token_current_relative:
            user_ty::relative_filename_preference_argument(do_nothing);
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
    switch (incomplete_change_number)
    {
    default:
        break;

    case 1:
        return complete_change_number(pp, 1 << cstate_state_being_developed);

    case 2:
        return complete_change_number(pp, 1 << cstate_state_completed);
    }

    //
    // If we need to complete a branch number, we have the project now.
    //
    if (incomplete_branch)
        return complete_project_branch(pp, 1);

    //
    // If we need to complete a delta number, we have the project now,
    // and the branch to use.
    //
    if (incomplete_delta)
    {
        if (branch)
            pp = pp->find_branch(branch);
        return complete_project_delta(pp);
    }

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // locate change data
    //
    if (independent)
    {
        //
        // create a bogus change
        //
        cp = change_alloc(pp, project_next_change_number(pp, 1));
        change_bind_new(cp);
        cp->bogus = 1;
    }
    else
    {
        if (!change_number)
            change_number = up->default_change(pp);
        cp = change_alloc(pp, change_number);
        change_bind_existing(cp);
    }

    //
    // Figure out whether we are using base relative file names, or
    // current directory relative file names.
    //
    baserel =
        (
            independent
        ||
            (
                up->relative_filename_preference
                (
                    uconf_relative_filename_preference_current
                )
            ==
                uconf_relative_filename_preference_base
            )
        );

    //
    // We are going to complete a project file name.
    // Just, not one already in the change.
    //
    return complete_project_file(cp, baserel, overwriting);
}


static command_vtbl_ty vtbl =
{
    destructor,
    completion_get,
    sizeof(command_ty),
    "aecp",
};


command_ty *
command_aecp()
{
    return command_new(&vtbl);
}


// vim: set ts=8 sw=4 et :
