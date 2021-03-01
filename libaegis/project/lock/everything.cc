//
//      aegis - project change supervisor
//      Copyright (C) 2006-2009, 2011, 2012 Peter Miller
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

#include <libaegis/change/branch.h>
#include <libaegis/project.h>


void
project::lock_prepare_everything()
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
        change::pointer cp = change_alloc(this, cn);
        change_bind_existing(cp);
        if (cp->is_a_branch())
        {
            project *pp2 = bind_branch(cp);
            pp2->lock_prepare_everything();
            // Do not free this project (change), or it will segfault
            // later.  This, of couse, a memory leak.  Once the project
            // smart pointers are being used universally, this will no
            // longer be a problem.
        }
        else if (!cp->is_completed())
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


// vim: set ts=8 sw=4 et :
