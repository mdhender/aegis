//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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

#include <change/branch.h>
#include <error.h> // for assert
#include <gonzo.h>
#include <os.h>
#include <project/history.h>
#include <trace.h>
#include <sub.h>
#include <user.h>


void
project_bind_new(project_ty *pp)
{
    int		    um;
    user_ty	    *up;
    cstate_history_ty *h;

    //
    // make sure does not already exist
    //
    trace(("project_bind_new()\n{\n"));
    if (gonzo_project_home_path_from_name(pp->name))
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "Name", pp->name);
	fatal_intl(scp, i18n("project $name exists"));
	// NOTREACHED
	sub_context_delete(scp);
    }

    //
    // allocate data structures
    //
    assert(!pp->pstate_data);
    assert(!pp->pstate_path);
    pp->is_a_new_file = 1;
    pp->pstate_data = (pstate_ty *)pstate_type.alloc();
    pp->pstate_data->next_test_number = 1;
    os_become_orig_query(&pp->uid, &pp->gid, &um);

    pp->pcp = change_alloc(pp, TRUNK_CHANGE_NUMBER);
    change_bind_new(pp->pcp);
    change_branch_new(pp->pcp);
    project_umask_set(pp, um);

    //
    // The new change is in the 'being developed'
    // state, and will be forever.
    //
    up = user_executing(pp);
    h = change_history_new(pp->pcp, up);
    h->what = cstate_history_what_new_change;
    h = change_history_new(pp->pcp, up);
    h->what = cstate_history_what_develop_begin;
    user_free(up);
    pp->pcp->cstate_data->state = cstate_state_being_developed;
    trace(("}\n"));
}
