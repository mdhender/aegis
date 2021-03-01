/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 John Darrington
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
 * MANIFEST: functions to manipulate history_labels
 */

#include <change.h>
#include <project.h>
#include <change/env_set.h>
#include <change/branch.h>
#include <error.h>	/* for assert */
#include <os.h>
#include <sub.h>
#include <user.h>
#include <trace.h>


void
change_run_history_label_command(cp, src, label)
    change_ty       *cp;
    fstate_src      src;
    string_ty       *label;
{
    pconf           pconf_data;
    string_ty       *the_command;
    string_ty       *hp;
    sub_context_ty  *scp;

    /*
     * Label the history.
     * Current directory will be set to the
     * base of the history tree.  All of the substitutions described
     * in aesub(5) are avaliable.  In addition:
     *
     * ${History}
     *      absolute path of history file
     * ${Edit}
     *      edit number
     * ${Label}
     *      label
     */
    trace(("change_run_history_label_command(cp = %8.8lX, filename = \"%s\", "
	"label = \"%s\")\n{\n", (long)cp, src->file_name->str_text,
	label->str_text));
    assert(cp->reference_count >= 1);

    pconf_data = change_pconf_get(cp, 0);
    if
    (
	!pconf_data->history_label_command
    ||
	!pconf_data->history_label_command->str_length
    )
    {
	trace(("}\n"));
	return;
    }

    /*
     * Get the path of the history file.
     */
    hp = project_history_path_get(cp->pp);
    scp = sub_context_new();
    sub_var_set_format(scp, "History", "%S/%S", hp, src->file_name);
    sub_var_set_string(scp, "Edit", src->edit->revision);
    sub_var_set_string(scp, "Label", label);

    the_command = pconf_data->history_label_command;
    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);
    change_env_set(cp, 0);
    project_become(cp->pp);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK, hp);
    project_become_undo();
    str_free(the_command);
    trace(("}\n"));
}
