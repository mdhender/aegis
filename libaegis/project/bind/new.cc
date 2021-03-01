//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate news
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
    int		    um;
    user_ty	    *up;
    cstate_history_ty *h;

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
    os_become_orig_query(&uid, &gid, &um);

    pcp = change_alloc(this, TRUNK_CHANGE_NUMBER);
    change_bind_new(pcp);
    change_branch_new(pcp);
    project_umask_set(this, um);

    //
    // The new change is in the 'being developed'
    // state, and will be forever.
    //
    up = user_executing(this);
    h = change_history_new(pcp, up);
    h->what = cstate_history_what_new_change;
    h = change_history_new(pcp, up);
    h->what = cstate_history_what_develop_begin;
    user_free(up);
    pcp->cstate_data->state = cstate_state_being_developed;
    trace(("}\n"));
}
