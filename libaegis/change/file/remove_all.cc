//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate remove_alls
//

#include <libaegis/change/file.h>
#include <common/error.h> // for assert
#include <common/symtab.h>
#include <common/trace.h>


void
change_file_remove_all(change_ty *cp)
{
    trace(("change_file_remove_all(cp = %08lX)\n{\n", (long)cp));
    fstate_ty *fstate_data = change_fstate_get(cp);
    assert(fstate_data->src);
    assert(cp->fstate_stp);
    if (fstate_data->src->length)
    {
	fstate_src_list_type.free(fstate_data->src);
	fstate_data->src = (fstate_src_list_ty *)fstate_src_list_type.alloc();
    }
    symtab_free(cp->fstate_stp);
    cp->fstate_stp = symtab_alloc(0);
    symtab_free(cp->fstate_uuid_stp);
    cp->fstate_uuid_stp = symtab_alloc(0);
    trace(("}\n"));
}
