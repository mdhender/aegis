//
// aegis - project change supervisor
// Copyright (C) 1999-2008, 2011, 2012 Peter Miller
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

#include <common/debug.h>
#include <common/error.h>
#include <common/nstring/list.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>


void
change::search_path_get(string_list_ty *wlp, bool resolve)
{
    wlp->clear();
    if (bogus)
    {
        // Absolutely no reason given as to why we skip the immediate
        // project and go on to the ancestors.  If you figure it out,
        // let me know.  I'm sure I had a good reason at the time.
        if (!pp->is_a_trunk())
            pp->parent_get()->search_path_get(wlp, resolve);
        return;
    }
    switch (cstate_get()->state)
    {
#ifndef DEBUG
    default:
        this_is_a_bug();
        break;
#endif

    case cstate_state_completed:
    case cstate_state_awaiting_development:
        pp->search_path_get(wlp, resolve);
        break;

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
        {
            string_ty *s = change_development_directory_get(this, resolve);
            wlp->push_back(s);
            pp->search_path_get(wlp, resolve);
        }
        break;

    case cstate_state_being_integrated:
        wlp->push_back(change_integration_directory_get(this, resolve));
        if (!pp->is_a_trunk())
            pp->parent_get()->search_path_get(wlp, resolve);
        break;
    }
}


void
change::search_path_get(nstring_list &result, bool resolve)
{
    result.clear();
    if (bogus)
    {
        // Absolutely no reason given as to why we skip the immediate
        // project and go on to the ancestors.  If you figure it out,
        // let me know.  I'm sure I had a good reason at the time.
        if (!pp->is_a_trunk())
            pp->parent_get()->search_path_get(result, resolve);
        return;
    }
    switch (cstate_get()->state)
    {
#ifndef DEBUG
    default:
        this_is_a_bug();
        break;
#endif

    case cstate_state_completed:
    case cstate_state_awaiting_development:
        pp->search_path_get(result, resolve);
        break;

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
        {
            nstring s(change_development_directory_get(this, resolve));
            result.push_back(s);
            pp->search_path_get(result, resolve);
        }
        break;

    case cstate_state_being_integrated:
        {
            nstring s(change_integration_directory_get(this, resolve));
            result.push_back(s);
            if (!pp->is_a_trunk())
                pp->parent_get()->search_path_get(result, resolve);
        }
        break;
    }
}


// vim: set ts=8 sw=4 et :
