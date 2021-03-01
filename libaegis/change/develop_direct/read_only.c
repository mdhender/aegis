/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002, 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate read_onlys
 */

#include <ac/stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <change.h>
#include <change/develop_direct/read_only.h>
#include <dir.h>
#include <os.h>
#include <sub.h>
#include <undo.h>


static void
func(void *arg, dir_walk_message_ty msg, string_ty *path, struct stat *st)
{
    switch (msg)
    {
    case dir_walk_dir_before:
    case dir_walk_special:
    case dir_walk_symlink:
	break;

    case dir_walk_dir_after:
    case dir_walk_file:
	if (st->st_mode & 0222)
	{
	    undo_chmod(path, st->st_mode & 07777);
	    os_chmod_errok(path, st->st_mode & 07555);
	}
	break;
    }
}


void
change_development_directory_chmod_read_only(change_ty *cp)
{
    cstate_ty       *cstate_data;
    string_ty       *dd;

    cstate_data = change_cstate_get(cp);
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_completed:
	return;

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
    case cstate_state_being_integrated:
	break;
    }

    change_verbose(cp, 0, i18n("making dev dir read only"));

    change_developer_become(cp);
    dd = change_development_directory_get(cp, 0);
    dir_walk(dd, func, cp);
    change_developer_become_undo();
}
