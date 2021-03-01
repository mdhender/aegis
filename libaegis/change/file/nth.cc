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

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/change/file.h>
#include <libaegis/change/file/list_get.h>


fstate_src_ty *
change_file_nth(change::pointer cp, size_t n, view_path_ty as_view_path)
{
    fstate_src_ty   *src_data;
    string_list_ty  *wlp;

    //
    // Special case the absence of a view path, it's slightly faster.
    //
    trace(("change_file_nth(cp = %p, n = %ld)\n{\n", cp, (long)n));
    if (as_view_path == view_path_first)
    {
        fstate_ty       *fstate_data;

        fstate_data = cp->fstate_get();
        assert(fstate_data->src);
        if (n >= fstate_data->src->length)
        {
            trace(("return NULL;\n"));
            trace(("}\n"));
            return 0;
        }
        src_data = fstate_data->src->list[n];
        trace(("return %p;\n", src_data));
        trace(("}\n"));
        return src_data;
    }

    // do not free wlp, it's cached
    wlp = change_file_list_get(cp, as_view_path);

    if (n < wlp->nstrings)
    {
        trace(("%s\n", wlp->string[n]->str_text));
        src_data = cp->file_find(nstring(wlp->string[n]), as_view_path);

        //
        // If this assert fails, it means that change_file_list_get and
        // change_file_find are interpreting the view path differently.
        //
        assert(src_data);
    }
    else
        src_data = 0;
    trace(("return %p;\n", src_data));
    trace(("}\n"));
    return src_data;
}

// vim: set ts=8 sw=4 et :
