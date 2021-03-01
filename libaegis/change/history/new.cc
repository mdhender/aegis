//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2008, 2012 Peter Miller
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

#include <common/now.h>
#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/user.h>


cstate_history_ty *
change_history_new(change::pointer cp, user_ty::pointer up)
{
    cstate_ty       *cstate_data;
    cstate_history_ty *history_data;
    cstate_history_ty **history_data_p;
    meta_type *type_p = 0;

    trace(("change_history_new(cp = %p)\n{\n", cp));
    assert(cp->reference_count >= 1);
    cstate_data = cp->cstate_get();
    assert(cstate_data->history);
    history_data_p =
        (cstate_history_ty **)
        cstate_history_list_type.list_parse(cstate_data->history, &type_p);
    assert(type_p == &cstate_history_type);
    history_data = (cstate_history_ty *)cstate_history_type.alloc();
    *history_data_p = history_data;
    history_data->when = now();
    history_data->who = str_copy(up->name().get_ref());
    trace(("return %p;\n", history_data));
    trace(("}\n"));
    return history_data;
}


// vim: set ts=8 sw=4 et :
