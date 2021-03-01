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
change_file_remove(change::pointer cp, string_ty *file_name)
{
    fstate_ty       *fstate_data;
    size_t          j;
    fstate_src_ty   *src_data;

    trace(("change_file_remove(cp = %p, file_name = \"%s\")\n{\n",
        cp, file_name->str_text));
    fstate_data = cp->fstate_get();
    assert(fstate_data->src);
    assert(cp->fstate_stp);
    cp->fstate_stp->remove(file_name);
    for (j = 0; j < fstate_data->src->length; ++j)
    {
        src_data = fstate_data->src->list[j];
        if (!str_equal(src_data->file_name, file_name))
            continue;
        if (src_data->uuid)
            cp->fstate_uuid_stp->remove(src_data->uuid);
        fstate_src_type.free(src_data);
        fstate_data->src->list[j] =
            fstate_data->src->list[--fstate_data->src->length];
        break;
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
