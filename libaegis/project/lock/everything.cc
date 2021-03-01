//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller;
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
// MANIFEST: implementation of the project_ty::lock_prepare_everything method
//

#include <libaegis/change/branch.h>
#include <libaegis/project.h>


void
project_ty::lock_prepare_everything()
{
    //
    // Lock everything that's easy to lock.
    //
    project_baseline_read_lock_prepare(this);
    project_history_lock_prepare(this);
    pstate_lock_prepare();

    //
    // Also lock all active changes, and recurse into all active branches.
    //
    for (long j = 0; ; ++j)
    {
	long cn = 0;
	if (!change_branch_change_nth(pcp, j, &cn))
	    break;
	change_ty *cp = change_alloc(this, cn);
	change_bind_existing(cp);
	if (change_is_a_branch(cp))
	{
    	    project_ty *pp2 = bind_branch(cp);
	    pp2->lock_prepare_everything();
            // Do not free this project (change), or it will segfault
            // later.  This, of couse, a memory leak.  Once the project
            // smart pointers are being used universally, this will no
            // longer be a problem.
	}
	else if (!change_is_completed(cp))
	{
	    change_cstate_lock_prepare(cp);
            // Do not free this change, or it will segfault later.
            // This, of couse, a memory leak.  One the change smart
            // pointers are being used universally, this will no longer
            // be a problem.
	}
	else
	{
	    change_free(cp);
	}
    }
}
