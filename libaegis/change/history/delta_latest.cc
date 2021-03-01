//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2008 Peter Miller
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


long
change_history_delta_latest(change::pointer cp)
{
	cstate_ty       *cstate_data;
	cstate_branch_history_list_ty *h;

	cstate_data = cp->cstate_get();
	if (!cstate_data->branch)
		return 0;
	h = cstate_data->branch->history;
	if (!h)
		return 0;
	if (!h->length)
		return 0;
	return h->list[h->length - 1]->delta_number;
}
