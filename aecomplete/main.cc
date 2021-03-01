//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
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
// MANIFEST: bash auto completion
//

#include <common/ac/stdlib.h>

#include <common/arglex.h>
#include <aecomplete/command.h>
#include <aecomplete/complete.h>
#include <common/env.h>
#include <common/language.h>
#include <libaegis/os.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <aecomplete/shell.h>


int
main(int argc, char **argv)
{
    shell_ty        *sh;
    command_ty      *cmd;
    complete_ty     *cop;

    //
    // initialize everything
    //
    resource_limits_init();
    arglex_init(argc, argv, 0);
    os_become_init_mortal();
    env_initialize();
    language_init();

    //
    // Work out what shell is talking to us.
    //
    sh = shell_find();

    //
    // Work out what command we are completing.
    //
    cmd = command_find(shell_command_get(sh));

    //
    // Ask the command what it wants completed.
    //
    cop = command_completion_get(cmd);

    //
    // Perform the completion.
    //
    complete_perform(cop, sh);
    complete_delete(cop);
    shell_delete(sh);

    //
    // Report success.
    //
    quit(0);
    return 0;
}
