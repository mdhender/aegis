//
// aegis - project change supervisor
// Copyright (C) 1999, 2002-2008, 2011, 2012 Peter Miller
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

#include <common/nstring/list.h>
#include <libaegis/change.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/project/file.h>


void
project::search_path_get(nstring_list &result, bool resolve)
{
    //
    // do NOT call change_search_path, it will make a mess
    //
    for
    (
        project *ppp = this;
        ppp;
        ppp = (ppp->is_a_trunk() ? 0 : ppp->parent_get())
    )
    {
        cstate_ty *cstate_data = ppp->change_get()->cstate_get();
        switch (cstate_data->state)
        {
        case cstate_state_awaiting_development:
        case cstate_state_completed:
            break;

        case cstate_state_being_developed:
        case cstate_state_awaiting_review:
        case cstate_state_being_reviewed:
        case cstate_state_awaiting_integration:
        case cstate_state_being_integrated:
            result.push_back(nstring(ppp->baseline_path_get(resolve)));
            break;
        }
    }
}


void
project::search_path_get(string_list_ty *wlp, bool resolve)
{
    //
    // do NOT call change_search_path, it will make a mess
    //
    for
    (
        project *ppp = this;
        ppp;
        ppp = (ppp->is_a_trunk() ? 0 : ppp->parent_get())
    )
    {
        cstate_ty *cstate_data = ppp->change_get()->cstate_get();
        switch (cstate_data->state)
        {
        case cstate_state_awaiting_development:
        case cstate_state_completed:
            break;

        case cstate_state_being_developed:
        case cstate_state_awaiting_review:
        case cstate_state_being_reviewed:
        case cstate_state_awaiting_integration:
        case cstate_state_being_integrated:
            wlp->push_back(ppp->baseline_path_get(resolve));
            break;
        }
    }
}


// vim: set ts=8 sw=4 et :
