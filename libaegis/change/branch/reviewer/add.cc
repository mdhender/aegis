//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2003-2008, 2012 Peter Miller
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
change_branch_reviewer_add(change::pointer cp, string_ty *usrnam)
{
    cstate_ty       *cstate_data;
    meta_type         *type_p;
    string_ty       **spp;
    cstate_branch_reviewer_list_ty *lp;
    size_t          j;

    trace(("change_branch_reviewer_add(cp = %p, "
        "usrnam = \"%s\")\n{\n", cp, usrnam->str_text));
    cstate_data = cp->cstate_get();
    assert(cstate_data->branch);
    if (!cstate_data->branch->reviewer)
    {
        cstate_data->branch->reviewer =
            (cstate_branch_reviewer_list_ty *)
            cstate_branch_reviewer_list_type.alloc();
    }
    lp = cstate_data->branch->reviewer;

    //
    // make sure we don't have her already
    //
    for (j = 0; j < lp->length; ++j)
    {
        if (str_equal(usrnam, lp->list[j]))
        {
            trace(("}\n"));
            return;
        }
    }

    //
    // append her to the list
    //
    spp =
        (string_ty **)
        cstate_branch_reviewer_list_type.list_parse
        (
            cstate_data->branch->reviewer,
            &type_p
        );
    assert(type_p == &string_type);
    *spp = str_copy(usrnam);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
