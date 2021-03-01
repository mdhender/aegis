//
// aegis - project change supervisor
// Copyright (C) 2002-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
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

#include <libaegis/change.h>
#include <libaegis/change/env_set.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


void
change_run_develop_begin_undo_command(change::pointer cp, user_ty::pointer up)
{
    pconf_ty        *pconf_data;
    string_ty       *the_command;
    string_ty       *dir;

    assert(!lock_active());
    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 0);
    if (!pconf_data->develop_begin_undo_command)
        return;

    the_command = pconf_data->develop_begin_undo_command;
    the_command = substitute(0, cp, the_command);

    change_env_set(cp, 0);
    user_ty::become scoped(up);

    //
    // The reason this code calls os_curdir and instead of calling
    // change_development_directory_get is that the development
    // directory is gone by the time this is called.
    //
    dir = os_curdir();

    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, dir);
    str_free(the_command);
}


// vim: set ts=8 sw=4 et :
