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


bool
change_branch_integrator_query(change::pointer cp, string_ty *name)
{
    trace(("change_branch_integrator_query(cp = %p, name = \"%s\")\n{\n",
        cp, name->str_text));
    cstate_ty *cstate_data = cp->cstate_get();
    assert(cstate_data->branch);
    if (!cstate_data->branch->integrator)
    {
        cstate_data->branch->integrator =
            (cstate_branch_integrator_list_ty *)
            cstate_branch_integrator_list_type.alloc();
    }
    cstate_branch_integrator_list_ty *lp = cstate_data->branch->integrator;
    for (size_t j = 0; j < lp->length; ++j)
    {
        if (str_equal(lp->list[j], name))
        {
            trace(("return true;\n"));
            trace(("}\n"));
            return true;
        }
    }
    trace(("return false;\n"));
    trace(("}\n"));
    return false;
}


// vim: set ts=8 sw=4 et :
