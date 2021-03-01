//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/change/branch.h>
#include <common/error.h> // for assert
#include <libaegis/gonzo.h>
#include <libaegis/os.h>
#include <libaegis/project/history.h>
#include <common/trace.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


void
project_ty::bind_new()
{
    //
    // make sure does not already exist
    //
    trace(("project_ty::bind_new()\n{\n"));
    if (gonzo_project_home_path_from_name(name))
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "Name", name);
	fatal_intl(scp, i18n("project $name exists"));
	// NOTREACHED
	sub_context_delete(scp);
    }

    //
    // allocate data structures
    //
    assert(!pstate_data);
    assert(!pstate_path);
    is_a_new_file = true;
    pstate_data = (pstate_ty *)pstate_type.alloc();
    pstate_data->next_test_number = 1;
    int um = 0;
    os_become_orig_query(&uid, &gid, &um);

    um |= 022;
    if (um & 1)
        um |= 4;
    um &= 027;

    //
    // Create the project user from the details of the user who created
    // the project (the executing user).
    //
    trace(("uid %d, gid %d\n", uid, gid));
    up = user_ty::create(uid, gid);
    trace(("um %04o\n", um));
    up->umask_set(um);

    pcp = change_alloc(this, TRUNK_CHANGE_NUMBER);
    change_bind_new(pcp);
    change_branch_new(pcp);
    project_umask_set(this, um);

    //
    // The new change is in the 'being developed'
    // state, and will be forever.
    //
    trace(("mark\n"));
    cstate_history_ty *h = change_history_new(pcp, up);
    h->what = cstate_history_what_new_change;
    h = change_history_new(pcp, up);
    h->what = cstate_history_what_develop_begin;
    pcp->cstate_data->state = cstate_state_being_developed;
    trace(("}\n"));
}
