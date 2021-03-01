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

#include <libaegis/change.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/project.h>
#include <libaegis/user.h>

#include <aecomplete/command/ae_c.h>
#include <aecomplete/command/private.h>
#include <aecomplete/complete/change/number.h>


static void
destructor(command_ty *)
{
}


static complete_ty *
completion_get(command_ty *)
{
    project      *pp;

    //
    // Work out which project to use.
    //
    nstring project_name = user_ty::create()->default_project();
    pp = project_alloc(project_name.get_ref());
    pp->bind_existing();

    //
    // We are going to complete a change number.
    //
    // They could want it for just about anything, but we will only
    // suggest active changes, which is more likely to be useful.
    //
    return
        complete_change_number
        (
            pp,
            ~(
                (1 << cstate_state_awaiting_development)
            |
                (1 << cstate_state_completed)
            )
        );
}


static command_vtbl_ty vtbl =
{
    destructor,
    completion_get,
    sizeof(command_ty),
    "ae_c",
};


command_ty *
command_ae_c()
{
    return command_new(&vtbl);
}


// vim: set ts=8 sw=4 et :
