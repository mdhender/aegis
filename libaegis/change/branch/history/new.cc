//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2003-2008, 2012 Peter Miller
//      Copyright (C) 2008, 2009 Walter Franzini
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

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/change/branch.h>


void

change_branch_history_new(change::pointer cp, long delta_number,
    long change_number, string_ty *uuid, time_t when, bool is_a_branch)
{
    cstate_ty       *cstate_data;
    cstate_branch_history_ty *hp;
    cstate_branch_history_ty **hpp;
    meta_type         *type_p;

    trace(("change_branch_history_new(cp = %p, delta_number = %ld, "
        "change_number = %ld)\n{\n", cp, delta_number, change_number));
    cstate_data = cp->cstate_get();
    assert(cstate_data->branch);
    if (!cstate_data->branch->history)
    {
        cstate_data->branch->history =
            (cstate_branch_history_list_ty *)
            cstate_branch_history_list_type.alloc();
    }
    hpp =
        (cstate_branch_history_ty **)
        cstate_branch_history_list_type.list_parse
        (
            cstate_data->branch->history,
            &type_p
        );
    assert(type_p == &cstate_branch_history_type);
    hp = (cstate_branch_history_ty *)cstate_branch_history_type.alloc();
    *hpp = hp;
    hp->delta_number = delta_number;
    hp->change_number = change_number;
    if (uuid)
        hp->uuid = str_copy(uuid);
    hp->when = when;
    if (is_a_branch)
        hp->is_a_branch = cstate_branch_history_is_a_branch_yes;
    else
        hp->is_a_branch = cstate_branch_history_is_a_branch_no;
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
