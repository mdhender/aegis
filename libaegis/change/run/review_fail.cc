//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/change/env_set.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>


void
change::run_review_fail_notify_command()
{
    trace(("change::run_review_fail_notify_command(this = %08lX)\n{\n",
        (long)this));

    //
    // make sure there is one
    //
    assert(!lock_active());
    assert(reference_count >= 1);
    string_ty *the_command = project_review_fail_notify_command_get(pp);
    if (!the_command || !the_command->str_length)
    {
        trace(("}\n"));
        return;
    }

    //
    // notify the review has failed
    //	(it could be mail, or an internal bulletin board, etc)
    // it happens after the data is written and the locks are released,
    // so we don't much care it the command fails!
    //
    // In doing it after the locks are released,
    // the lists will be accurate (e.g. list of files in change).
    //
    // All of the substitutions described in aesub(5) are available.
    //
    sub_context_ty sc;
    the_command = sc.substitute(this, the_command);

    //
    // execute the command
    //
    string_ty *dd = change_development_directory_get(this, 0);
    change_env_set(this, 0);
    change_become(this);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, dd);
    str_free(the_command);
    change_become_undo(this);
    trace(("}\n"));
}
