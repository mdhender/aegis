//
//	aegis - project change supervisor
//	Copyright (C) 2007 Walter Franzini
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

#include <common/error.h>       // for assert
#include <common/symtab.h>
#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


fstate_ty *
change_pfstate_get(change::pointer cp)
{
    //
    // make sure the change state has been read in,
    // in case its src field needed to be converted.
    // (also to ensure lock_sync has been called for both)
    //
    trace(("change_pfstate_get(cp = %08lX)\n{\n", (long)cp));

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
	cp->pfstate_stp = symtab_alloc(cp->pfstate_data->src->length);
	cp->pfstate_uuid_stp = symtab_alloc(cp->pfstate_data->src->length);
	for (size_t j = 0; j < cp->pfstate_data->src->length; ++j)
	{
	    fstate_src_ty *p = cp->pfstate_data->src->list[j];
	    symtab_assign(cp->pfstate_stp, p->file_name, p);
	    if (p->uuid && (!p->move || p->action != file_action_remove))
		symtab_assign(cp->pfstate_uuid_stp, p->uuid, p);
	}
    }
    trace(("return %08lX;\n", (long)cp->pfstate_data));
    trace(("}\n"));
    return cp->pfstate_data;
}
