//
// aegis - project change supervisor
// Copyright (C) 1999, 2002-2008, 2011, 2012 Peter Miller
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
#include <libaegis/change.h>


fstate_src_ty *
change::file_find_fuzzy(const nstring &file_name, view_path_ty)
{
    trace(("change::file_find_fuzzy(fn = %s)\n{\n",
        file_name.quote_c().c_str()));
    fstate_get();
    assert(fstate_stp);
    nstring best_file_name = fstate_stp->query_fuzzy(file_name);
    fstate_src_ty *best =
        (
            best_file_name.empty()
        ?
            0
        :
            (fstate_src_ty *)fstate_stp->query(best_file_name)
        );
    trace(("return %p;\n", best));
    trace(("}\n"));
    return best;
}


// vim: set ts=8 sw=4 et :
