//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2006 Peter Miller;
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
// MANIFEST: functions to manipulate integra_passs
//

#include <libaegis/change.h>
#include <libaegis/change/env_set.h>
#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
change_run_integrate_pass_notify_command(change_ty *cp)
{
    sub_context_ty	*scp;
    string_ty	*the_command;
    string_ty	*bl;

    //
    // make sure there is one
    //
    trace(("change_run_integrate_pass_notify_command(cp = %08lX)\n{\n",
	(long)cp));
    assert(cp->reference_count >= 1);
    the_command = project_integrate_pass_notify_command_get(cp->pp);
    if (!the_command || !the_command->str_length)
	goto done;

    //
    // notify the integrate has passed
    //	(it could be mail, or an internal bulletin board, etc)
    // it happens after the data is written and the locks are released,
    // so we don't much care it the command fails!
    //
    // In doing it after the locks are released,
    // the lists will be accurate (e.g. list of files in change).
    //
    // All of the substitutions described in aesub(5) are available.
    //
    scp = sub_context_new();
    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);

    //
    // execute the command
    //
    bl = cp->pp->baseline_path_get();
    change_env_set(cp, 0);
    project_become(cp->pp);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, bl);
    project_become_undo();
    str_free(the_command);

    //
    // here for all exits
    //
    done:
    trace(("}\n"));
}
