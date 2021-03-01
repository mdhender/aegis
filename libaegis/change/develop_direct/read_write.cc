//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate read_writes
//

#include <ac/stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cstate.h>
#include <change.h>
#include <change/develop_direct/read_write.h>
#include <dir.h>
#include <os.h>
#include <project/history.h>
#include <sub.h>
#include <undo.h>


static void
func(void *arg, dir_walk_message_ty msg, string_ty *path, struct stat *st)
{
    mode_t          mode;
    int             mask;

    switch (msg)
    {
    case dir_walk_dir_after:
    case dir_walk_special:
    case dir_walk_symlink:
	break;

    case dir_walk_dir_before:
    case dir_walk_file:
	mask = *(int *)arg;
	mode = 07777 & ((st->st_mode | 0666) & ~mask);
	if ((st->st_mode & 07777) != mode)
	{
	    undo_chmod(path, st->st_mode & 07777);
	    os_chmod_errok(path, mode);
	}
	break;
    }
}


void
change_development_directory_chmod_read_write(change_ty *cp)
{
    cstate_ty       *cstate_data;
    string_ty       *dd;
    int             mask;

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

    change_verbose(cp, 0, i18n("making dev dir writable"));

    dd = change_development_directory_get(cp, 0);
    mask = project_umask_get(cp->pp);
    change_developer_become(cp);
    dir_walk(dd, func, &mask);
    change_developer_become_undo();
}
