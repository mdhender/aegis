//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate lock_syncs
//

#include <change.h>
#include <change/lock_sync.h>
#include <lock.h>
#include <str_list.h>
#include <symtab.h>


void
change_lock_sync(change_ty *cp)
{
    if (cp->lock_magic != lock_magic())
	change_lock_sync_forced(cp);
}


void
change_lock_sync_forced(change_ty *cp)
{
    size_t          j;

    cp->lock_magic = lock_magic();

    if (cp->cstate_data && !cp->cstate_is_a_new_file)
    {
	cstate_type.free(cp->cstate_data);
	cp->cstate_data = 0;
    }
    if (cp->fstate_data && !cp->fstate_is_a_new_file)
    {
	fstate_type.free(cp->fstate_data);
	cp->fstate_data = 0;
    }
    if (cp->fstate_stp)
    {
	symtab_free(cp->fstate_stp);
	cp->fstate_stp = 0;
    }

    for (j = 0; j < view_path_MAX; ++j)
    {
	if (cp->file_list[j])
	{
	    string_list_delete(cp->file_list[j]);
	    cp->file_list[j] = 0;
	}
    }
}
