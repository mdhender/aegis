//
//      aegis - project change supervisor
//      Copyright (C) 2003-2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <libaegis/change.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>


long
project_delta_number_to_change_number(project *pp, long delta_number)
{
    change::pointer cp;
    cstate_ty       *cstate_data;
    sub_context_ty  *scp;

    cp = pp->change_get();
    cstate_data = cp->cstate_get();
    if (cstate_data->branch)
    {
        cstate_branch_history_list_ty *csbhlp;

        csbhlp = cstate_data->branch->history;
        if (csbhlp)
        {
            size_t          j;

            for (j = 0; j < csbhlp->length; ++j)
            {
                cstate_branch_history_ty *hp;

                hp = csbhlp->list[j];
                if (hp->delta_number == delta_number)
                    return hp->change_number;
            }
        }
    }

    scp = sub_context_new();
    sub_var_set_long(scp, "Name", delta_number);
    project_fatal(pp, scp, i18n("no delta $name"));
    // NOTREACHED
    return 0;
}


// vim: set ts=8 sw=4 et :
