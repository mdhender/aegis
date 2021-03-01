//
//      aegis - project change supervisor
//      Copyright (C) 2001-2008, 2012 Peter Miller
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
change_history_delta_name_add(change::pointer cp, long delta_number,
    string_ty *delta_name)
{
    cstate_ty       *cstate_data;
    cstate_branch_history_list_ty *h;
    size_t          j;
    cstate_branch_history_ty *he;

    trace(("change_history_delta_name_add(cp = %p, delta_number = %ld, "
        "delta_name = \"%s\")\n{\n", cp, delta_number,
        delta_name->str_text));
    cstate_data = cp->cstate_get();
    assert(cstate_data->branch);
    h = cstate_data->branch->history;
    assert(h);
    assert(h->length);
    for (j = 0; j < h->length; ++j)
    {
        meta_type *type_p = 0;
        string_ty       **addr_p;

        he = h->list[j];
        if (he->delta_number != delta_number)
            continue;

        //
        // add the name to the selected history entry
        //
        if (!he->name)
        {
            he->name =
                (cstate_branch_history_name_list_ty *)
                cstate_branch_history_name_list_type.alloc();
        }
        addr_p =
            (string_ty **)
            cstate_branch_history_name_list_type.list_parse(he->name, &type_p);
        assert(type_p == &string_type);
        *addr_p = str_copy(delta_name);
        break;
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
