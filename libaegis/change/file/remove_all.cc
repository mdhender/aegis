//
// aegis - project change supervisor
// Copyright (C) 1999, 2003-2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
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

#include <common/symtab.h>
#include <common/trace.h>
#include <libaegis/change/file.h>


void
change_file_remove_all(change::pointer cp)
{
    trace(("change_file_remove_all(cp = %p)\n{\n", cp));
    fstate_ty *fstate_data = cp->fstate_get();
    assert(fstate_data->src);
    assert(cp->fstate_stp);
    if (fstate_data->src->length)
    {
        fstate_src_list_type.free(fstate_data->src);
        fstate_data->src = (fstate_src_list_ty *)fstate_src_list_type.alloc();
    }
    delete cp->fstate_stp;
    cp->fstate_stp = new symtab_ty;
    delete cp->fstate_uuid_stp;
    cp->fstate_uuid_stp = new symtab_ty;
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
