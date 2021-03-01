/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate is_completeds
 */

#include <change.h>
#include <cstate.h>
#include <error.h>


int
change_is_completed(change_ty *cp)
{
    cstate          cstate_data;

    if (cp->bogus)
	return 0;
    cstate_data = change_cstate_get(cp);
    assert(cstate_data);
    if (!cstate_data)
	return 0;
    return (cstate_data->state == cstate_state_completed);
}
