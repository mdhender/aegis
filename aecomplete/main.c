/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002, 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: bash auto completion
 */

#include <ac/stdlib.h>

#include <arglex.h>
#include <command.h>
#include <complete.h>
#include <env.h>
#include <language.h>
#include <os.h>
#include <r250.h>
#include <shell.h>
#include <str.h>


int
main(int argc, char **argv)
{
    shell_ty        *sh;
    command_ty      *cmd;
    complete_ty     *cop;

    /*
     * initialize everything
     */
    arglex_init(argc, argv, 0);
    r250_init();
    os_become_init_mortal();
    str_initialize();
    env_initialize();
    language_init();

    /*
     * Work out what shell is talking to us.
     */
    sh = shell_find();

    /*
     * Work out what command we are completing.
     */
    cmd = command_find(shell_command_get(sh));

    /*
     * Ask the command what it wants completed.
     */
    cop = command_completion_get(cmd);

    /*
     * Perform the completion.
     */
    complete_perform(cop, sh);
    complete_delete(cop);
    shell_delete(sh);

    /*
     * Report success.
     */
    exit(0);
    return 0;
}
