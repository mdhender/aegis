//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: implementation of the project_invento_walk class
//

#include <change/branch.h>
#include <error.h> // for assert
#include <now.h>
#include <project.h>
#include <project/history.h>
#include <project/invento_walk.h>
#include <str_list.h>
#include <trace.h>


static bool
project_is_completed_branch(project_ty *pp)
{
    change_ty *cp = project_change_get(pp);
    return change_is_completed(cp);
}


static time_t
project_completion_timestamp(project_ty *pp)
{
    change_ty *cp = project_change_get(pp);
    return change_completion_timestamp(cp);
}


static void
project_change_inventory_get(project_ty *pp, change_functor &result,
    time_t time_limit, bool maximum)
{
    trace(("project_change_inventory_get(pp = %08lX)\n{\n", (long)pp));

    if (maximum)
    {
	//
	// Pull out all of the changes' UUIDs.
	//
	for (size_t j = 0; ; ++j)
	{
	    long cn = 0;
	    if (!project_change_nth(pp, j, &cn))
		break;

	    change_ty *cp = change_alloc(pp, cn);
	    change_bind_existing(cp);
	    time_t when = change_completion_timestamp(cp);
	    if (when <= time_limit)
	    {
		if (change_was_a_branch(cp))
		{
		    if (result.include_branches())
			result(cp);

		    //
		    // Recurse on branches.
		    //
		    project_ty *sub_pp =
			project_bind_branch(pp, change_copy(cp));
		    project_change_inventory_get
		    (
			sub_pp,
			result,
			time_limit,
			maximum
		    );
		    project_free(sub_pp);
		}
		else
		{
		    //
		    // Call the functor.
		    //
		    result(cp);
		}
	    }
	    change_free(cp);
	}
    }
    else
    {
	//
	// Pull out all of the completed changes' UUIDs.
	//
	for (size_t j = 0; ; ++j)
	{
	    //
	    // Get the changes in integration order.  Because this is the
	    // historical order, there is no need to sort once we have the
	    // full set of changes.
	    //
	    // This is especially important when there are parallel branches
	    // in operation.  They will have been integrated in a specific
	    // order, and the first branches changes come first, then the
	    // seconds branbches changes come second.
	    //
	    long cn = 0;
	    long dn = 0;
	    string_list_ty name;
	    if (!project_history_nth(pp, j, &cn, &dn, &name))
		break;

            //
            // Once a change is past the time limit, all later ones will
            // be, too.
	    //
	    change_ty *cp = change_alloc(pp, cn);
	    change_bind_existing(cp);
	    assert(change_is_completed(cp));
	    time_t when = change_completion_timestamp(cp);
	    if (when > time_limit)
	    {
		change_free(cp);
		break;
	    }

	    if (change_was_a_branch(cp))
	    {
		if (result.include_branches())
		    result(cp);

		//
		// Recurse on completed branches.
		//
		project_ty *sub_pp = project_bind_branch(pp, change_copy(cp));
		project_change_inventory_get(sub_pp, result, time_limit, false);
		project_free(sub_pp);
	    }
	    else
	    {
		//
		// Call the functor.
		//
		result(cp);
	    }
	    change_free(cp);
	}
    }
    trace(("}\n"));
}


static void
project_change_inventory_getr(project_ty *pp, change_functor &result,
    time_t limit, time_t maximum)
{
    if (pp->parent)
	project_change_inventory_getr(pp->parent, result, limit, false);
    project_change_inventory_get(pp, result, limit, maximum);
}


static void
project_inventory_walk(project_ty *pp, change_functor &result, time_t limit,
    bool maximum)
{
    trace(("project_inventory_walk(pp = %08lX, limit = %ld)\n{\n", (long)pp,
	(long)limit));
    if (project_is_completed_branch(pp))
    {
	time_t time_limit = project_completion_timestamp(pp);
	if (time_limit > limit)
	    time_limit = limit;
	project_ty *ppp = pp;
	while (ppp->parent)
	{
	    ppp = ppp->parent;
	    if (!project_is_completed_branch(ppp))
		break;
	}
	// The trunk is never completed.
	project_change_inventory_getr(ppp, result, time_limit, false);
    }
    else
    {
	project_change_inventory_getr(pp, result, limit, maximum);
    }
    trace(("}\n"));
}


void
project_inventory_walk(project_ty *pp, change_functor &result, time_t limit)
{
    trace(("project_inventory_walk(pp = %08lX)\n{\n", (long)pp));
    project_inventory_walk(pp, result, limit, result.all_changes());
    trace(("}\n"));
}


void
project_inventory_walk(project_ty *pp, change_functor &result)
{
    trace(("project_inventory_walk(pp = %08lX)\n{\n", (long)pp));
    time_t when = project_completion_timestamp(pp);
    project_inventory_walk(pp, result, when);
    trace(("}\n"));
}
