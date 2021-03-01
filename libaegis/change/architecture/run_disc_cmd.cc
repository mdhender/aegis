//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate run_disc_cmds
//

#include <change.h>
#include <change/directory.h>
#include <change/env_set.h>
#include <error.h> // for assert
#include <os.h>
#include <pconf.h>
#include <project.h>
#include <sub.h>
#include <user.h>


string_ty *
change_run_architecture_discriminator_command(change_ty *cp)
{
    pconf_ty        *pconf_data;
    string_ty       *the_command;
    string_ty       *dir;
    user_ty         *up;
    string_ty       *result;
    static int      loop_detect;

    //
    // There are a couple of ways this function can be called recursively.
    // They are mentioned below.
    //
    if (loop_detect)
	return 0;
    ++loop_detect;
    assert(cp->reference_count >= 1);

    //
    // Get the command to be executed.  If there is none, return nothing.
    //
    pconf_data = change_pconf_get(cp, 0);
    the_command = pconf_data->architecture_discriminator_command;
    if (!the_command || !the_command->str_length)
    {
	--loop_detect;
	return 0;
    }

    //
    // Substitute the command.
    //
    // It is entirely possible (stupid, but possible) that they will use
    // $arch inside the command substitution we are about to perform.
    // (This will cause a recursion which loop_detect will deal with.)
    //
    the_command = substitute(0, cp, the_command);

    //
    // Set the usual environment variables.
    // (This will cause a recursion which loop_detect will deal with.)
    //
    change_env_set(cp, 0);

    //
    // Run the command and collect the output.
    //
    dir = change_directory_get(cp, 0);
    up = user_executing(cp->pp);
    user_become(up);
    result =
	os_execute_slurp
	(
	    the_command,
	    OS_EXEC_FLAG_NO_INPUT | OS_EXEC_FLAG_SILENT,
	    dir
	);
    user_become_undo();
    str_free(the_command);
    user_free(up);

    //
    // Only return some output if it isn't empty.
    //
    --loop_detect;
    if (result && !result->str_length)
    {
	str_free(result);
	return 0;
    }
    return result;
}
