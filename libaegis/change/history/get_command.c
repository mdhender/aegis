/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate get_commands
 */

#include <change.h>
#include <change/env_set.h>
#include <error.h> /* for assert */
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


void
change_run_history_get_command(cp, file_name, edit_number, output_file, up)
	change_ty	*cp;
	string_ty	*file_name;
	string_ty	*edit_number;
	string_ty	*output_file;
	user_ty		*up;
{
	sub_context_ty	*scp;
	string_ty	*dir;
	cstate		cstate_data;
	string_ty	*the_command;
	pconf		pconf_data;

	/*
	 * If the edit numbers differ, extract the
	 * appropriate edit from the baseline (use the
	 * history-get-command) into a file in /tmp
	 *
	 * Current directory set to the base of the history tree.
	 * All of the substitutions described in aesub(5) are available;
	 * in addition
	 *
	 * ${History}
	 *	history file
	 *
	 * ${Edit}
	 *	edit number
	 *
	 * ${Output}
	 *	output file
	 */
	trace(("change_run_history_get_command(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	scp = sub_context_new();
	sub_var_set
	(
		scp,
		"History",
		"%S/%S",
		project_history_path_get(cp->pp),
		file_name
	);
	sub_var_set(scp, "Edit", "%S", edit_number);
	sub_var_set(scp, "Output", "%S", output_file);
	sub_var_set(scp, "1", "${history}");
	sub_var_set(scp, "2", "${edit}");
	sub_var_set(scp, "3", "${output}");
	pconf_data = change_pconf_get(cp, 1);

	the_command = pconf_data->history_get_command;
	if (!the_command)
	{
		sub_context_ty	*scp2;

		scp2 = sub_context_new();
		sub_var_set(scp2, "File_Name", "%s", THE_CONFIG_FILE);
		sub_var_set(scp2, "FieLD_Name", "history_get_command");
		change_fatal
		(
			cp,
			scp2,
			i18n("$filename: contains no \"$field_name\" field")
		);
		sub_context_delete(scp2);
	}
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	/*
	 * run the command as the current user
	 * (always output is to /tmp)
	 */
	cstate_data = change_cstate_get(cp);
	dir = project_history_path_get(cp->pp);
	change_env_set(cp, 0);
	user_become(up);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dir);
	user_become_undo();
	str_free(the_command);
	trace((/*{*/"}\n"));
}
