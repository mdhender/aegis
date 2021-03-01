//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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

#include <libaegis/project/change/walk.h>
#include <libaegis/project/history.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/functor.h>
#include <common/str_list.h>
#include <common/trace.h>


void
project_change_walk(project_ty *pp, change_functor &func)
{
    trace(("project_change_walk(pp = %08lX)\n{\n", (long)pp));
    time_t earliest = func.earliest();
    time_t latest = func.latest();
    bool include_branches = func.include_branches();
    bool recurse_branches = func.recurse_branches();
    bool all_changes = func.all_changes();
    if (all_changes || recurse_branches)
    {
	//
	// Walk all change sets
	//
	for (size_t j = 0; ; ++j)
	{
	    long cn = 0;
	    if (!project_change_nth(pp, j, &cn))
		break;

	    change::pointer cp = change_alloc(pp, cn);
	    change_bind_existing(cp);
	    time_t when = change_completion_timestamp(cp);
	    if (change_was_a_branch(cp))
	    {
		if (include_branches)
		{
		    if (when >= earliest && when <= latest)
			func(cp);
		}
		if (recurse_branches)
		{
		    project_ty *sub_pp = pp->bind_branch(change_copy(cp));
		    project_change_walk(sub_pp, func);
		    project_free(sub_pp);
		}
	    }
	    else if (all_changes || cp->is_completed())
	    {
		if (when >= earliest && when <= latest)
		{
		    //
		    // Call the functor.
		    //
		    func(cp);
		}
	    }
	    change_free(cp);
	}
    }
    else
    {
	//
	// Get the changes in integration order.  Because this is the
	// historical order, there is no need to sort once we have the
	// full set of changes.
	//
	for (size_t j = 0; ; ++j)
	{
	    long cn = 0;
	    long dn = 0;
	    string_list_ty name;
	    if (!project_history_nth(pp, j, &cn, &dn, &name))
		break;

	    change::pointer cp = change_alloc(pp, cn);
	    change_bind_existing(cp);
	    time_t when = change_completion_timestamp(cp);
	    if (change_was_a_branch(cp))
	    {
		if (include_branches)
		{
		    if (when >= earliest && when <= latest)
			func(cp);
		}
		if (recurse_branches)
		{
		    project_ty *sub_pp = pp->bind_branch(change_copy(cp));
		    project_change_walk(sub_pp, func);
		    project_free(sub_pp);
		}
	    }
	    else
	    {
		if (when >= earliest && when <= latest)
		{
		    //
		    // Call the functor.
		    //
		    func(cp);
		}
	    }
	    change_free(cp);
	}
    }
    trace(("}\n"));
}
