//
// aegis - project change supervisor
// Copyright (C) 2008, 2012 Peter Miller
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


void
change::run_develop_begin_early_command(user_ty::pointer up)
{
    assert(lock_active());
    assert(reference_count >= 1);
    pconf_ty *pcp = change_pconf_get(this, 0);
    string_ty *the_command = pcp->develop_begin_early_command;
    if (!the_command || !the_command->str_length)
        return;
    the_command = substitute(0, this, the_command);
    string_ty *dir = change_development_directory_get(this, 1);
    change_env_set(this, 0);
    user_ty::become scoped(up);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, dir);
    str_free(the_command);
}


// vim: set ts=8 sw=4 et :
