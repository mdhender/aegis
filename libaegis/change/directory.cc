//
//	aegis - project change supervisor
//	Copyright (C) 2003-2008 Peter Miller
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

#include <libaegis/change.h>
#include <libaegis/change/directory.h>
#include <libaegis/project/directory.h>


string_ty *
change_directory_get(change::pointer cp, int resolve)
{
    cstate_ty       *cstate_data;

    cstate_data = cp->cstate_get();
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
	break;

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
	return change_development_directory_get(cp, resolve);

    case cstate_state_being_integrated:
	return change_integration_directory_get(cp, resolve);

    case cstate_state_completed:
	break;
    }
    return  project_directory_get(cp->pp, resolve);
}
