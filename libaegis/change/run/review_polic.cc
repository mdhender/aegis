//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: implementation of the change_run_review_polic class
//

#include <change.h>
#include <change/run/review_polic.h>
#include <error.h> // for assert
#include <os.h>
#include <str_list.h>
#include <sub.h>
#include <symtab.h>
#include <user.h>


int
change_run_review_policy_command(change_ty *cp, user_ty *up)
{
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
    user_become(up);
    int result = os_execute_retcode(the_command, flags, dir);
    user_become_undo();

    str_free(the_command);
    return result;
}
