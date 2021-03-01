//
//	aegis - project change supervisor
//	Copyright (C) 2005 Walter Franzini;
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
// MANIFEST: implementation of the change_when_get class
//

#include <libaegis/change.h>


time_t
change_when_get(change_ty *cp, cstate_history_what_ty what)
{
    time_t when = 0;
    cstate_ty *cstate_data = change_cstate_get(cp);
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
