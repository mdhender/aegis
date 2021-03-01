//
// aegis - project change supervisor
// Copyright (C) 2005-2008, 2011, 2012 Peter Miller
// Copyright (C) 2008 Walter Franzini
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

#include <libaegis/change/file.h>
#include <libaegis/change/pfs.h>
#include <libaegis/project.h>
#include <libaegis/project/pattr/get.h>
#include <libaegis/undo.h>


void
change_pfs_write(change::pointer cp)
{
    //
    // The ".pfs" file contains the same stuff as the branch ".fs"
    // file with the exception of some transient data that are
    // stripped out.
    //
    // The delta's "psf" file is immutable, which should give very
    // good file-system cache behaviour.
    //
    // If we ever split the project file state into individual files
    // (because for most operations it will be faster) then this
    // function will need to be re-implemented.
    //
    pconf_ty *pconf_data = change_pconf_get(cp, 1);
    if (pconf_data->cache_project_file_list_for_each_delta)
    {
        change::pointer pcp = cp->pp->change_get();
        fstate_ty *fstate_data = fstate_clone(pcp->fstate_get());
        assert(fstate_data);
        fstate_src_list_ty *src = fstate_data->src;
        assert(src);

        //
        // We need to purge the pfstate content of transient data:
        // * reset the locked_by field, since it will be out of date
        //   at read time;
        // * remove fake transparent entries.
        //
        for (size_t j = 0; j < src->length; ++j)
        {
            fstate_src_ty *p_src_data = src->list[j];

            //
            // Reset transient fields.
            //
            if (p_src_data->locked_by)
                p_src_data->locked_by = 0;

            //
            //
            //
            switch (p_src_data->action)
            {
            case file_action_insulate:
                assert(0);
                // fallthrough

            case file_action_create:
            case file_action_remove:
            case file_action_modify:
                assert(!p_src_data->about_to_be_created_by);
                assert(!p_src_data->about_to_be_copied_by);
                break;

            case file_action_transparent:
                if
                (
                    p_src_data->about_to_be_copied_by
                ||
                    p_src_data->about_to_be_created_by
                )
                {
                    //
                    // This entry is a fake transparent, remove it
                    // from the list.
                    //
                    for (size_t k = j + 1; k < src->length; ++k)
                        src->list[k - 1] = src->list[k];
                    fstate_src_type.free(p_src_data);
                    --j;
                    src->length--;
                }
                break;
            }
        }

        nstring fn(change_pfstate_filename_get(cp));

        change_become(cp);
        fstate_write_file
        (
            fn,
            fstate_data,
            project_compress_database_get(cp->pp)
        );
        undo_unlink_errok(fn);
        change_become_undo(cp);
    }
}


// vim: set ts=8 sw=4 et :
