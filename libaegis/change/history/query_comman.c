/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate query_commans
 */

#include <change.h>
#include <change/env_set.h>
#include <error.h> /* for assert */
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>


string_ty *
change_run_history_query_command(cp, filename)
	change_ty	*cp;
	string_ty	*filename;
{
	sub_context_ty	*scp;
	string_ty	*hp;
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*result;

	/*
	 * Ask the history file what its edit number is.  We use this
	 * method because the string returned is essentially random,
	 * between different history programs.  Only ever executed in
	 * the ``being integrated'' state.  Current directory will be
	 * set to the base of the history tree.  All of the
	 * substitutions described in aesub(5) are available.  In
	 * addition
	 *
	 * ${History}
	 *	absolute path of history file
	 */
	trace(("change_run_history_query_command(cp = %8.8lX, \
filename = \"%s\")\n{\n"/*}*/, (long)cp, filename->str_text));
	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 1);
	hp = project_history_path_get(cp->pp);
	scp = sub_context_new();
	sub_var_set_format(scp, "History", "%S/%S", hp, filename);
	sub_var_set_charstar(scp, "1", "${history}");
	the_command = pconf_data->history_query_command;
	if (!the_command)
	{
		sub_context_ty	*scp2;

		scp2 = sub_context_new();
		sub_var_set_charstar(scp2, "File_Name", THE_CONFIG_FILE);
		sub_var_set_charstar(scp2, "FieLD_Name", "history_query_command");
		change_fatal
		(
			cp,
			scp2,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp2);
	}
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	change_env_set(cp, 0);
	project_become(cp->pp);
	result =
		os_execute_slurp
		(
			the_command,
			OS_EXEC_FLAG_NO_INPUT,
			hp
		);
	project_become_undo();
	str_free(the_command);
	if (!result->str_length)
		fatal_intl(0, i18n("history_query_command return empty"));
	trace(("return \"%s\";\n", result->str_text));
	trace((/*{*/"}\n"));
	return result;
}
