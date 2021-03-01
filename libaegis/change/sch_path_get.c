/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999-2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate sch_path_gets
 */

#include <change/branch.h>
#include <change/file.h>
#include <error.h>
#include <project.h>
#include <project/file.h>
#include <str_list.h>


void
change_search_path_get(change_ty *cp, string_list_ty *wlp, int resolve)
{
    cstate_ty       *cstate_data;
    project_ty      *ppp;
    string_ty       *s;

    string_list_constructor(wlp);
    if (cp->bogus)
    {
	ppp = cp->pp->parent;
	if (ppp)
	    project_search_path_get(ppp, wlp, resolve);
	return;
    }
    cstate_data = change_cstate_get(cp);
    switch (cstate_data->state)
    {
    default:
	this_is_a_bug();
	break;

    case cstate_state_completed:
    case cstate_state_awaiting_development:
	project_search_path_get(cp->pp, wlp, resolve);
	break;

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
	s = change_development_directory_get(cp, resolve);
	string_list_append(wlp, s);
	project_search_path_get(cp->pp, wlp, resolve);
	break;

    case cstate_state_being_integrated:
	string_list_append(wlp, change_integration_directory_get(cp, resolve));
	ppp = cp->pp->parent;
	if (ppp)
	    project_search_path_get(ppp, wlp, resolve);
	break;
    }
}
