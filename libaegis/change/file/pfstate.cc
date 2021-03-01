//
//	aegis - project change supervisor
//	Copyright (C) 2007-2009 Walter Franzini
//	Copyright (C) 2007, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>

#include <common/error.h>       // for assert
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
change_pfstate_get(change::pointer cp)
{
    //
    // make sure the change state has been read in,
    // in case its src field needed to be converted.
    // (also to ensure lock_sync has been called for both)
    //
    trace(("change_pfstate_get(cp = %p)\n{\n", cp));

    cp->cstate_get();

    if (!cp->pfstate_data)
    {
        string_ty *fn = change_pfstate_filename_get(cp);
	change_become(cp);
        if (os_exists(fn))
            cp->pfstate_data = fstate_read_file(fn);
	change_become_undo(cp);

        if (!cp->pfstate_data)
            return (fstate_ty*)0;

        pfimprove(cp->pfstate_data, fn, cp);
    }
    if (!cp->pfstate_data->src)
    {
	cp->pfstate_data->src =
            (fstate_src_list_ty *)fstate_src_list_type.alloc();
    }

    //
    // Create a couple of O(1) indexes.
    // This speeds up just about everything.
    //
    if (!cp->pfstate_stp)
    {
        assert(!cp->pfstate_uuid_stp);
	cp->pfstate_stp = new symtab_ty(cp->pfstate_data->src->length);
	cp->pfstate_uuid_stp = new symtab_ty(cp->pfstate_data->src->length);
	for (size_t j = 0; j < cp->pfstate_data->src->length; ++j)
	{
	    fstate_src_ty *p = cp->pfstate_data->src->list[j];
	    cp->pfstate_stp->assign(p->file_name, p);
	    if (p->uuid && (!p->move || p->action != file_action_remove))
		cp->pfstate_uuid_stp->assign(p->uuid, p);
	}
    }
    trace(("return %p;\n", cp->pfstate_data));
    trace(("}\n"));
    return cp->pfstate_data;
}
