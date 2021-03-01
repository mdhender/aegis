//
//	aegis - project change supervisor
//	Copyright (C) 2001-2008 Peter Miller
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
#include <common/error.h> // for assert
#include <libaegis/project.h>


time_t
change_history_delta_to_timestamp(project_ty *pp, long delta_number)
{
	cstate_ty       *cstate_data;
	cstate_branch_history_list_ty *hl;
	size_t		j;
	change::pointer cp;

	cp = pp->change_get();
	cstate_data = cp->cstate_get();
	if (!cstate_data->branch)
		return (time_t)-1;
	if (delta_number == 0)
	{
		//
		// special case, meaning
		// "when the branch was created"
		//
		if (cstate_data->history && cstate_data->history->length)
			return cstate_data->history->list[0]->when;
		return (time_t)-1;
	}
	hl = cstate_data->branch->history;
	if (!hl)
		return (time_t)-1;
	for (j = 0; j < hl->length; ++j)
	{
		cstate_branch_history_ty *bh;
		change::pointer cp2;
		time_t		result;

		bh = hl->list[j];
		assert(bh);
		if (!bh)
			continue;
		if (bh->delta_number != delta_number)
			continue;
		cp2 = change_alloc(pp, bh->change_number);
		change_bind_existing(cp2);
		result = change_completion_timestamp(cp2);
		change_free(cp2);
		return result;
	}
	return (time_t)-1;
}
