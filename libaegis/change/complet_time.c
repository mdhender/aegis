/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate complet_times
 */

#include <change/branch.h>
#include <error.h> /* for assert */


time_t
change_completion_timestamp(cp)
    change_ty       *cp;
{
    cstate          cstate_data;
    time_t          now;

    cstate_data = change_cstate_get(cp);
    if (cstate_data->state == cstate_state_completed)
    {
	cstate_history  chp;
	cstate_history_list chlp;

	chlp = cstate_data->history;
	assert(chlp);
	assert(chlp->length);
	chp = chlp->list[chlp->length - 1];
	assert(chp);
	return chp->when;
    }
    time(&now);
    return now;
}