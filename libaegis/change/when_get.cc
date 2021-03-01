//
//	aegis - project change supervisor
//	Copyright (C) 2005 Walter Franzini
//	Copyright (C) 2007, 2008 Peter Miller
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


time_t
change_when_get(change::pointer cp, cstate_history_what_ty what)
{
    time_t when = 0;
    cstate_ty *cstate_data = cp->cstate_get();
    cstate_history_list_ty *history = cstate_data->history;
    if (!history)
	return 0;
    for (size_t i = 0; i < history->length; ++i)
    {
	if (!history->list[i])
	    continue;
        if (history->list[i]->what != what)
            continue;
        when = history->list[i]->when;
    }
    return when;
}
