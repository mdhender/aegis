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

#include <libaegis/change/branch.h>
#include <common/fstrcmp.h>
#include <libaegis/sub.h>


long
change_history_change_by_delta(change::pointer cp, long delta_number)
{
    cstate_ty       *cstate_data;
    cstate_branch_history_list_ty *h;
    size_t	    j;
    sub_context_ty  *scp;

    cstate_data = cp->cstate_get();
    if (!cstate_data->branch)
	goto useless;
    h = cstate_data->branch->history;
    if (!h)
	goto useless;
    if (!h->length)
	goto useless;
    for (j = 0; j < h->length; ++j)
    {
	cstate_branch_history_ty *he;

	he = h->list[j];
	if (he->delta_number == delta_number)
	    return he->change_number;
    }

    useless:
    scp = sub_context_new();
    sub_var_set_long(scp, "Number", delta_number);
    change_fatal(cp, scp, i18n("no delta $number"));
    // NOTREACHED
    sub_context_delete(scp);
    return 0;
}
