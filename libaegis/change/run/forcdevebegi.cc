//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate forcdevebegis
//

#include <change.h>
#include <change/env_set.h>
#include <error.h> // for assert
#include <os.h>
#include <project/history.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


void
change_run_forced_develop_begin_notify_command(change_ty *cp, user_ty *up)
{
    string_ty       *the_command;
    string_ty       *dd;

    //
    // make sure there is one
    //
    trace(("change_run_forced_develop_begin_notify_command(cp = %8.8lX, "
	"up = %8.8lX)\n{\n", (long)cp, (long)up));
    assert(cp->reference_count >= 1);
    the_command = project_forced_develop_begin_notify_command_get(cp->pp);
    if (!the_command)
	goto done;

    //
    // notify the change is ready for review
    //	(it could be mail, or an internal bulletin board, etc)
    // it happens after the data is written and the locks are released,
    // so we don't much care if the command fails!
    //
    // All of the substitutions described in aesub(5) are available.
    //
    the_command = substitute(0, cp, the_command);

    //
    // execute the command
    //
    dd = change_development_directory_get(cp, 0);
    change_env_set(cp, 0);
    user_become(up);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, dd);
    user_become_undo();
    str_free(the_command);

    //
    // here for all exits
    //
    done:
    trace(("}\n"));
}
