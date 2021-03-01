//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate dev_beg_undos
//

#include <libaegis/change.h>
#include <libaegis/change/env_set.h>
#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


void
change_run_develop_begin_undo_command(change_ty *cp, user_ty *up)
{
    pconf_ty        *pconf_data;
    string_ty       *the_command;
    string_ty       *dir;

    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 0);
    if (!pconf_data->develop_begin_undo_command)
	return;

    the_command = pconf_data->develop_begin_undo_command;
    the_command = substitute(0, cp, the_command);
    dir = os_curdir();
    change_env_set(cp, 0);
    user_become(up);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, dir);
    user_become_undo();
    str_free(the_command);
}
