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

#include <common/trace.h>
#include <libaegis/change/file.h>
#include <libaegis/project/file.h>


fstate_src_ty *
project::file_find(const nstring &file_name, view_path_ty as_view_path)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return file_find(file_name.get_ref(), as_view_path);
}


fstate_src_ty *
project::file_find(string_ty *file_name, view_path_ty as_view_path)
{
    trace(("project::file_find(file_name = \"%s\", as_view_path = %s)\n{\n",
        file_name->str_text, view_path_ename(as_view_path)));
    change::pointer cp = change_get();
    fstate_src_ty *src_data =
        cp->file_find(nstring(file_name), view_path_first);
    if (src_data)
    {
        trace(("%s \"%s\"\n", file_action_ename(src_data->action),
            src_data->file_name->str_text));
        switch (as_view_path)
        {
        case view_path_first:
            //
            // This means that transparent and removed files
            // are returned.  No project file union, either.
            //
            break;

        case view_path_none:
            //
            // This means that transparent files are returned as
            // transparent, no processing or filtering of the list
            // is performed.
            //
            break;

        case view_path_extreme:
            //
            // This means that transparent files are resolved (the
            // underlying file is shown), and removed files are
            // omitted from the result.
            //
            switch (src_data->action)
            {
            case file_action_remove:
                trace(("return NULL;\n"));
                trace(("}\n"));
                return 0;

            case file_action_create:
            case file_action_modify:
            case file_action_insulate:
            case file_action_transparent:
#ifndef DEBUG
            default:
#endif
                if (src_data->deleted_by)
                {
                    trace(("return NULL;\n"));
                    trace(("}\n"));
                    return 0;
                }
                break;
            }
            // fall through...

        case view_path_simple:
            //
            // This means that transparent files are resolved
            // (the underlying file is shown), but removed files
            // are retained.
            //
            switch (src_data->action)
            {
            case file_action_transparent:
                goto deeper;

            case file_action_remove:
                //
                // Remember if the file has been removed.
                //
                break;

            case file_action_create:
            case file_action_modify:
            case file_action_insulate:
#ifndef DEBUG
            default:
#endif
                // should be file_action_remove
                assert(!src_data->deleted_by);
                if (src_data->deleted_by)
                    break;

                // should be file_action_transparent
                assert(!src_data->about_to_be_copied_by);
                if (src_data->about_to_be_copied_by)
                    goto deeper;

                // should be file_action_transparent
                assert(!src_data->about_to_be_created_by);
                if (src_data->about_to_be_created_by)
                    goto deeper;
                break;
            }
            break;
        }
        trace(("return %p;\n", src_data));
        trace(("}\n"));
        return src_data;
    }
    if (as_view_path == view_path_first)
    {
        //
        // This means that transparent and removed files
        // are returned.  No project file union, either.
        //
        trace(("return NULL;\n"));
        trace(("}\n"));
        return 0;
    }
    deeper:
    if (is_a_trunk())
    {
        trace(("return NULL;\n"));
        trace(("}\n"));
        return 0;
    }
    src_data = parent_get()->file_find(file_name, as_view_path);
    trace(("return %p;\n", src_data));
    trace(("}\n"));
    return src_data;
}


// vim: set ts=8 sw=4 et :
