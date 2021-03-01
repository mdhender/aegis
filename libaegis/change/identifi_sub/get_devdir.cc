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
// MANIFEST: implementation of the change_identifi_sub_get_devdir class
//

#include <libaegis/change.h>
#include <libaegis/change/identifi_sub.h>
#include <libaegis/cstate.h>


bool
change_identifier_subset::get_devdir()
{
    if (!devdir)
    {
	cstate_ty *cstate_data = change_cstate_get(get_cp());
	switch (cstate_data->state)
	{
	case cstate_state_awaiting_development:
	case cstate_state_completed:
	case cstate_state_being_integrated:
	    break;

	case cstate_state_being_developed:
	case cstate_state_awaiting_review:
	case cstate_state_being_reviewed:
	case cstate_state_awaiting_integration:
	    devdir = true;
	    break;
	}
    }
    return devdir;
}
