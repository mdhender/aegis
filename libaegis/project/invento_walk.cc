//
// aegis - project change supervisor
// Copyright (C) 2004-2009, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/now.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/change/branch.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/project/invento_walk.h>


static bool
project_is_completed_branch(project *pp)
{
    change::pointer cp = pp->change_get();
    return cp->is_completed();
}


static time_t
project_completion_timestamp(project *pp)
{
    change::pointer cp = pp->change_get();
    return cp->completion_timestamp();
}


static void
project_change_inventory_get(project *pp, change_functor &result,
    time_t time_limit, bool maximum)
{
    trace(("project_change_inventory_get(pp = %p)\n{\n", pp));

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

            change::pointer cp = change_alloc(pp, cn);
            change_bind_existing(cp);
            time_t when = cp->completion_timestamp();
            if (when <= time_limit)
            {
                if (cp->was_a_branch())
                {
                    if (result.include_branches())
                        result(cp);

                    //
                    // Recurse on branches.
                    //
                    project *sub_pp = pp->bind_branch(change_copy(cp));
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
            // seconds branches changes come second.
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
            change::pointer cp = change_alloc(pp, cn);
            change_bind_existing(cp);
            assert(cp->is_completed());
            time_t when = cp->completion_timestamp();
            if (when > time_limit)
            {
                change_free(cp);
                break;
            }

            if (cp->was_a_branch())
            {
                if (result.include_branches())
                    result(cp);

                //
                // Recurse on completed branches.
                //
                project *sub_pp = pp->bind_branch(change_copy(cp));
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
project_change_inventory_getr(project *pp, change_functor &result,
    time_t limit, time_t maximum)
{
    if (!pp->is_a_trunk())
        project_change_inventory_getr(pp->parent_get(), result, limit, false);
    project_change_inventory_get(pp, result, limit, maximum);
}


static void
project_inventory_walk(project *pp, change_functor &result, time_t limit,
    bool maximum)
{
    trace(("project_inventory_walk(pp = %p, limit = %ld)\n{\n", pp,
        (long)limit));
    if (project_is_completed_branch(pp))
    {
        time_t time_limit = project_completion_timestamp(pp);
        if (time_limit > limit)
            time_limit = limit;
        project *ppp = pp;
        while (!ppp->is_a_trunk())
        {
            ppp = ppp->parent_get();
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
project_inventory_walk(project *pp, change_functor &result, time_t limit)
{
    trace(("project_inventory_walk(pp = %p)\n{\n", pp));
    project_inventory_walk(pp, result, limit, result.all_changes());
    trace(("}\n"));
}


void
project_inventory_walk(project *pp, change_functor &result)
{
    trace(("project_inventory_walk(pp = %p)\n{\n", pp));
    time_t when = project_completion_timestamp(pp);
    project_inventory_walk(pp, result, when);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
