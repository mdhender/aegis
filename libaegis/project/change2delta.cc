//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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
#include <common/error.h> // for assert
#include <libaegis/project/history.h>
#include <libaegis/sub.h>


long
project_change_number_to_delta_number(project_ty *pp, long change_number)
{
    change::pointer cp;
    cstate_ty       *cstate_data;
    long	    result;

    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);
    cstate_data = cp->cstate_get();
    if (cstate_data->state != cstate_state_completed)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_long(scp, "Number", change_number);
	change_fatal(cp, scp, i18n("change $number not completed"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    result = cstate_data->delta_number;
    assert(result);
    change_free(cp);
    return result;
}
