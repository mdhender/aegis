/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate project_files
 */

#include <change.h>
#include <change/env_set.h>
#include <error.h> /* for assert */
#include <os.h>
#include <project/history.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


void
change_run_project_file_command(change_ty *cp, user_ty *up)
{
    sub_context_ty  *scp;
    cstate          cstate_data;
    pconf           pconf_data;
    string_ty       *the_command;
    string_ty       *dd;

    /*
     * make sure are sync'ed with project
     */
    trace(("change_run_project_file_command(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    cstate_data = change_cstate_get(cp);
    if
    (
	cstate_data->project_file_command_sync
    ==
	project_last_change_integrated(cp->pp)
    )
	goto done;
    cstate_data->project_file_command_sync =
	project_last_change_integrated(cp->pp);

    /*
     * make sure there is a project_file command
     */
    pconf_data = change_pconf_get(cp, 0);
    if (!pconf_data->project_file_command)
	goto done;

    /*
     * All of the substitutions described in aesub(5) are available.
     */
    scp = sub_context_new();
    sub_var_set_charstar(scp, "1", "${project}");
    sub_var_set_charstar(scp, "2", "${change}");
    sub_var_set_charstar(scp, "3", "${version}");
    sub_var_set_charstar(scp, "4", "${baseline}");
    the_command = pconf_data->project_file_command;
    the_command = substitute(scp,cp, the_command);
    sub_context_delete(scp);

    /*
     * execute the command
     */
    dd = change_development_directory_get(cp, 0);
    change_env_set(cp, 0);
    user_become(up);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT | OS_EXEC_FLAG_ERROK, dd);
    user_become_undo();
    str_free(the_command);

    /*
     * here for all exits
     */
    done:
    trace(("}\n"));
}
