//
//	aegis - project change supervisor
//	Copyright (C) 1999-2008 Peter Miller
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
#include <libaegis/change/file.h>
#include <common/error.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <common/str_list.h>


void
change_search_path_get(change::pointer cp, string_list_ty *wlp, int resolve)
{
    cstate_ty       *cstate_data;
    string_ty       *s;

    wlp->clear();
    if (cp->bogus)
    {
        // Absolutely no reason given as to why we skip the immediate
        // project and go on to the ancestors.  If you figure it out,
        // let me know.  I'm sure I had a good reason at the time.
	if (!cp->pp->is_a_trunk())
	    project_search_path_get(cp->pp->parent_get(), wlp, resolve);
	return;
    }
    cstate_data = cp->cstate_get();
    switch (cstate_data->state)
    {
#ifndef DEBUG
    default:
	this_is_a_bug();
	break;
#endif

    case cstate_state_completed:
    case cstate_state_awaiting_development:
	project_search_path_get(cp->pp, wlp, resolve);
	break;

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
	s = change_development_directory_get(cp, resolve);
	wlp->push_back(s);
	project_search_path_get(cp->pp, wlp, resolve);
	break;

    case cstate_state_being_integrated:
	wlp->push_back(change_integration_directory_get(cp, resolve));
	if (!cp->pp->is_a_trunk())
	    project_search_path_get(cp->pp->parent_get(), wlp, resolve);
	break;
    }
}
