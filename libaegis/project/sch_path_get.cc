//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate sch_path_gets
//

#include <change.h>
#include <cstate.h>
#include <project/file.h>
#include <str_list.h>


void
project_search_path_get(project_ty *pp, string_list_ty *wlp, int resolve)
{
    project_ty      *ppp;

    //
    // do NOT call change_search_path, it will make a mess
    //
    for (ppp = pp; ppp; ppp = ppp->parent)
    {
	change_ty       *cp;
	cstate_ty       *cstate_data;

	cp = project_change_get(pp);
	cstate_data = change_cstate_get(cp);
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
	    wlp->push_back(project_baseline_path_get(ppp, resolve));
	    break;
	}
    }
}
