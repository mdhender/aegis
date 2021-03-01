//
// aegis - project change supervisor
// Copyright (C) 1999, 2003-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <libaegis/change.h>


void
change_architecture_add(change::pointer cp, string_ty *name)
{
    assert(cp->reference_count >= 1);
    cstate_ty *cstate_data = cp->cstate_get();
    if (!cstate_data->architecture)
    {
        cstate_data->architecture =
            (cstate_architecture_list_ty *)
            cstate_architecture_list_type.alloc();
    }

    //
    // We must be careful to suppress duplicates, otherwise the
    // architecture prerequisites for state transitions are
    // unsatifiable.
    //
    for (size_t j = 0; j < cstate_data->architecture->length; ++j)
        if (str_equal(name, cstate_data->architecture->list[j]))
            return;

    meta_type *type_p = 0;
    string_ty **who_p =
        (string_ty **)
        cstate_architecture_list_type.list_parse
        (
            cstate_data->architecture,
            &type_p
        );
    assert(type_p == &string_type);
    *who_p = str_copy(name);
}


// vim: set ts=8 sw=4 et :
