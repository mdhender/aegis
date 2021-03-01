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
 * MANIFEST: functions to manipulate create_commas
 */

#include <change.h>
#include <change/env_set.h>
#include <change/file.h>
#include <error.h>
#include <os.h>
#include <project.h>
#include <str.h>
#include <sub.h>
#include <trace.h>


void
change_run_history_create_command(cp, filename)
	change_ty	*cp;
	string_ty	*filename;
{
	sub_context_ty	*scp;
	string_ty	*hp;
	string_ty	*s;
	pconf		pconf_data;
	string_ty	*the_command;

	/*
	 * Create a new history.  Only ever executed in the ``being
	 * integrated'' state.  Current directory will be set to the
	 * base of the history tree.  All of the substitutions described
	 * in aesub(5) are avaliable.  In addition:
	 *
	 * ${Input}
	 *	absolute path of source file
	 *
	 * ${History}
	 *	absolute path of history file
	 */
	trace(("change_run_history_create_command(cp = %8.8lX, \
filename = \"%s\")\n{\n"/*}*/, (long)cp, filename->str_text));
	assert(cp->reference_count >= 1);

	/*
	 * Get the path of the source file.
	 */
	scp = sub_context_new();
	s = change_file_path(cp, filename);
	sub_var_set(scp, "Input", "%S", s);
	str_free(s);

	/*
	 * Get the path of the history file.
	 */
	hp = project_history_path_get(cp->pp);
	sub_var_set(scp, "History", "%S/%S", hp, filename);

	/*
	 * Ancient compatibility.
	 * I sure hope no-one is still using this.
	 */
	sub_var_set(scp, "1", "${input}");
	sub_var_set(scp, "2", "${history}");

	/*
	 * Make sure the command has been set.
	 */
	pconf_data = change_pconf_get(cp, 1);
	the_command = pconf_data->history_create_command;
	if (!the_command)
	{
		sub_context_ty	*scp2;

		scp2 = sub_context_new();
		sub_var_set(scp2, "File_Name", "%s", THE_CONFIG_FILE);
		sub_var_set(scp2, "FieLD_Name", "history_create_command");
		change_fatal
		(
			cp,
			scp2,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp2);
	}

	/*
	 * Perform the substitutions and run the command.
	 */
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);
	change_env_set(cp, 0);
	project_become(cp->pp);
	os_mkdir_between(hp, filename, 02755);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, hp);
	project_become_undo();
	str_free(the_command);
	trace((/*{*/"}\n"));
}
