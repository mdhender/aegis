//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate review_passs
//

#include <libaegis/change.h>
#include <libaegis/change/env_set.h>
#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
change_run_review_pass_notify_command(change_ty *cp)
{
    sub_context_ty  *scp;
    string_ty       *the_command;
    string_ty       *dd;

    //
    // make sure there is one
    //
    trace(("change_run_review_pass_notify_command(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    the_command = project_review_pass_notify_command_get(cp->pp);
    if (!the_command || !the_command->str_length)
	goto done;

    //
    // notify the review has passed
    //	(it could be mail, or an internal bulletin board, etc)
    // it happens after the data is written and the locks are released,
    // so we don't much care if the command fails!
    //
    // All of the substitutions described in aesub(5) are available.
    //
    scp = sub_context_new();
    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);

    //
    // execute the command
    //
    dd = change_development_directory_get(cp, 0);
    change_env_set(cp, 0);
    change_become(cp);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, dd);
    str_free(the_command);
    change_become_undo();

    //
    // here for all exits
    //
    done:
    trace(("}\n"));
}
