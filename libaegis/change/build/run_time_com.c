/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2003 Peter Miller;
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
 * MANIFEST: functions to run the build_time_adjust_notify_command
 */

#include <change.h>
#include <change/env_set.h>
#include <error.h> /* for assert */
#include <os.h>
#include <project.h>
#include <sub.h>


void
change_run_build_time_adjust_notify_command(change_ty *cp)
{
    sub_context_ty  *scp;
    pconf_ty        *pconf_data;
    string_ty       *the_command;
    string_ty       *id;

    assert(cp->reference_count >= 1);
    assert(cp->cstate_data);
    /* happens during aeipass, but after state is set to completed */
    assert(cp->cstate_data->state == cstate_state_completed);
    pconf_data = change_pconf_get(cp, 1);
    assert(pconf_data);
    the_command = pconf_data->build_time_adjust_notify_command;
    if (!the_command)
	return;

    scp = sub_context_new();
    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);

    id = change_integration_directory_get(cp, 0);
    change_env_set(cp, 1);
    project_become(cp->pp);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, id);
    project_become_undo();
    str_free(the_command);
}
