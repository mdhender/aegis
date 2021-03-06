//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2002-2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/change/env_set.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


void
change::run_project_file_command(const user_ty::pointer &up)
{
    trace(("change::run_project_file_command(this = %p)\n{\n", this));
    assert(!lock_active());
    assert(reference_count >= 1);

    //
    // make sure there is a project_file command
    //
    pconf_ty *pcp = change_pconf_get(this, 0);
    if (!pcp->project_file_command)
    {
        trace(("}\n"));
        return;
    }

    //
    // All of the substitutions described in aesub(5) are available.
    //
    sub_context_ty sc;
    string_ty *the_command = pcp->project_file_command;
    the_command = sc.substitute(this, the_command);

    //
    // execute the command
    //
    string_ty *dd = change_development_directory_get(this, 0);
    change_env_set(this, 0);
    user_ty::become scoped(up);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT | OS_EXEC_FLAG_ERROK, dd);
    str_free(the_command);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
