//
//	aegis - project change supervisor
//	Copyright (C) 2003-2008 Peter Miller
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
#include <libaegis/change/directory.h>
#include <libaegis/change/env_set.h>
#include <libaegis/change.h>
#include <libaegis/os.h>
#include <libaegis/pconf.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


string_ty *
change_run_architecture_discriminator_command(change::pointer cp)
{
    pconf_ty        *pconf_data;
    string_ty       *the_command;
    string_ty       *dir;
    user_ty::pointer up;
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
    up = user_ty::create();
    {
        user_ty::become scoped(up);
        result =
            os_execute_slurp
            (
                the_command,
                OS_EXEC_FLAG_NO_INPUT | OS_EXEC_FLAG_SILENT,
                dir
            );
    }
    str_free(the_command);

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
