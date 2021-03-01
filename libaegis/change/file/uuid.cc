//
// aegis - project change supervisor
// Copyright (C) 2004-2008, 2011, 2012 Peter Miller
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
#include <libaegis/change/file.h> // delete when change_file_nth is refactored
#include <libaegis/change.h>


fstate_src_ty *
change::file_find_uuid(string_ty *uuid, view_path_ty view_path)
{
    //
    // See if we can do a fast lookup first.
    //
    assert(uuid);
    assert(this);
    if (view_path == view_path_first)
    {
        //
        // Make sure fstate has been read in and the symbol tables used
        // to speed up lookups have been initialised.
        //
        fstate_get();
        assert(fstate_uuid_stp);
        if (fstate_uuid_stp)
        {
            fstate_src_ty *src =
                (fstate_src_ty *)fstate_uuid_stp->query(uuid);
            if (src)
            {
                assert(src->action != file_action_remove || !src->move);
            }
            return src;
        }
    }

    //
    // Do it the slow way.
    //
    // FIXME: why not keep and index for each view path type?
    //
    for (size_t j = 0; ; ++j)
    {
        fstate_src_ty   *src;

        src = change_file_nth(this, j, view_path);
        if (!src)
            return 0;
        if (src->uuid && str_equal(uuid, src->uuid))
        {
            //
            // When a file is renamed, it appears in the change's file
            // list twice: once for the remove and again for the create.
            // Both entires have the same UUID.  Most of the time the
            // user is expecting the create side (with the new name) not
            // the remove side.
            //
            if (src->action != file_action_remove || !src->move)
                return src;
        }
    }
}


fstate_src_ty *
change::file_find_uuid(const nstring &uuid, view_path_ty view_path)
{
    return file_find_uuid(uuid.get_ref(), view_path);
}


// vim: set ts=8 sw=4 et :
