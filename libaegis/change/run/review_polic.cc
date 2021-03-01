//
// aegis - project change supervisor
// Copyright (C) 2004-2008, 2012 Peter Miller
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

#include <common/str_list.h>
#include <common/symtab.h>
#include <libaegis/change.h>
#include <libaegis/change/run/review_polic.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


int
change_run_review_policy_command(change::pointer cp, user_ty::pointer up)
{
    assert(lock_active());
    assert(cp->reference_count >= 1);
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    assert(pconf_data);
    string_ty *the_command = pconf_data->review_policy_command;

    // Default is to return zero (goto awaiting_integration)
    if (!the_command || !the_command->str_length)
        return 0;

    //
    // Make the necessary substitutions into the command.
    //
    the_command = substitute(0, cp, the_command);

    //
    // Run the command and collect the exit status.
    //
    int flags = OS_EXEC_FLAG_NO_INPUT;
    string_ty *dir = change_development_directory_get(cp, 1);
    user_ty::become scoped(up);
    int result = os_execute_retcode(the_command, flags, dir);
    str_free(the_command);
    return result;
}


// vim: set ts=8 sw=4 et :
