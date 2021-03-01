//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2002-2008, 2012 Peter Miller
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
#include <libaegis/change.h>
#include <libaegis/change/cstate/improve.h>


void
change_bind_new(change::pointer cp)
{
        trace(("change_bind_new(cp = %p)\n{\n", cp));
        assert(cp->reference_count >= 1);
        assert(!cp->cstate_data);
        cp->cstate_is_a_new_file = 1;
        cp->cstate_data = (cstate_ty *)cstate_type.alloc();
        cp->fstate_is_a_new_file = 1;
        cp->fstate_data = (fstate_ty *)fstate_type.alloc();
        change_cstate_improve(cp->cstate_data);
        trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
