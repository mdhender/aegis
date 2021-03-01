//
// aegis - project change supervisor
// Copyright (C) 2007-2009 Walter Franzini
// Copyright (C) 2007, 2008, 2011, 2012 Peter Miller
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
#include <common/ac/string.h>

#include <common/symtab.h>
#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


static void
pfimprove (fstate_ty *pfstate_data, string_ty *, change::pointer)
{
    assert(pfstate_data);
    if (!pfstate_data->src)
        return;

    fstate_src_list_ty *src = pfstate_data->src;

    for (size_t j = 0; j < src->length; ++j)
    {
        fstate_src_ty *p_src_data = src->list[j];
        assert(p_src_data);

        //
        // The locked_by field will be outdated at the time the
        // pfstate file will be read, clear it.
        //
        if (p_src_data->locked_by)
            p_src_data->locked_by = 0;

        switch(p_src_data->action)
        {
        case file_action_insulate:
            assert(0);
            // fallthrough

        case file_action_create:
        case file_action_modify:
        case file_action_remove:
#ifndef DEBUG
        default:
#endif
            //
            // The about_to_be_{copied,created}_by fields will be
            // outdated at the time the pfstate file will be read,
            // clear it.
            //
            if (p_src_data->about_to_be_copied_by)
                p_src_data->about_to_be_copied_by = 0;
            if (p_src_data->about_to_be_created_by)
                p_src_data->about_to_be_created_by = 0;
            continue;

        case file_action_transparent:
            //
            // We consider a transparent entry to be genuine only
            // if the about_to_be_{copied,created} fields are not
            // set.  This does not leave out corner cases since the
            // sequence aemt, aecp only sets locked_by.
            //
            if
            (
                !p_src_data->about_to_be_copied_by
            &&
                !p_src_data->about_to_be_created_by
            )
                continue;
            break;
        }

        //
        // This entry is a fake transparent, remove it from the list.
        //
        for (size_t k = j; k < src->length; ++k)
            src->list[k] = src->list[k+1];

        fstate_src_type.free(p_src_data);
        --j;
        src->length--;
    }
}


fstate_ty *
change::pfstate_get(void)
{
    //
    // make sure the change state has been read in,
    // in case its src field needed to be converted.
    // (also to ensure lock_sync has been called for both)
    //
    trace(("change::pfstate_get()\n{\n"));

    cstate_get();

    if (!pfstate_data)
    {
        string_ty *fn = change_pfstate_filename_get(this);
        change_become(this);
        if (os_exists(fn))
            pfstate_data = fstate_read_file(fn);
        change_become_undo(this);

        if (!pfstate_data)
            return (fstate_ty*)0;

        pfimprove(pfstate_data, fn, this);
    }
    if (!pfstate_data->src)
    {
        pfstate_data->src =
            (fstate_src_list_ty *)fstate_src_list_type.alloc();
    }

    //
    // Create a couple of O(1) indexes.
    // This speeds up just about everything.
    //
    if (!pfstate_stp)
    {
        assert(!pfstate_uuid_stp);
        pfstate_stp = new symtab_ty(pfstate_data->src->length);
        pfstate_uuid_stp = new symtab_ty(pfstate_data->src->length);
        for (size_t j = 0; j < pfstate_data->src->length; ++j)
        {
            fstate_src_ty *p = pfstate_data->src->list[j];
            pfstate_stp->assign(p->file_name, p);
            if (p->uuid && (!p->move || p->action != file_action_remove))
                pfstate_uuid_stp->assign(p->uuid, p);
        }
    }
    trace(("return %p;\n", pfstate_data));
    trace(("}\n"));
    return pfstate_data;
}


// vim: set ts=8 sw=4 et :
