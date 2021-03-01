//
// aegis - project change supervisor
// Copyright (C) 1999, 2002-2006, 2008, 2011, 2012 Peter Miller
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

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/project/file.h>


fstate_src_ty *
project::file_nth(size_t n, view_path_ty as_view_path)
{
    trace(("project::file_nth(this = %p, n = %ld)\n{\n", this, (long)n));
    // do not delete wlp, it's cached
    string_list_ty *wlp = file_list_get(as_view_path);

    fstate_src_ty *src_data = 0;
    if (n < wlp->nstrings)
    {
        trace(("%s\n", wlp->string[n]->str_text));
        src_data = file_find(wlp->string[n], as_view_path);

        //
        // If this assert fails, it means that project::file_list_get and
        // project::file_find are interpreting the view path differently.
        // That would be a bug.
        //
        assert(src_data);
    }
    trace(("return %p;\n", src_data));
    trace(("}\n"));
    return src_data;
}


// vim: set ts=8 sw=4 et :
