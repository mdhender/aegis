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
 * MANIFEST: functions to manipulate change_files
 */

#include <change.h>
#include <change/env_set.h>
#include <error.h> /* for assert */
#include <os.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


void
change_run_change_file_command(cp, wlp, up)
	change_ty	*cp;
	string_list_ty	*wlp;
	user_ty		*up;
{
	sub_context_ty	*scp;
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*the_files;
	string_ty	*dd;

	trace(("change_run_change_file_command(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 0);
	if (!pconf_data->change_file_command)
		goto ret;

	scp = sub_context_new();
	the_files = wl2str(wlp, 0, 32767, (char *)0);
	sub_var_set(scp, "File_List", "%S", the_files);
	sub_var_optional(scp, "File_List");
	str_free(the_files);
	sub_var_set(scp, "1", "${project}");
	sub_var_set(scp, "2", "${change}");
	sub_var_set(scp, "3", "${version}");
	sub_var_set(scp, "4", "${baseline}");
	sub_var_set(scp, "5", "${file_list}");
	the_command = pconf_data->change_file_command;
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	dd = change_development_directory_get(cp, 0);
	change_env_set(cp, 0);
	user_become(up);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT | OS_EXEC_FLAG_ERROK, dd);
	user_become_undo();
	str_free(the_command);
	ret:
	trace((/*{*/"}\n"));
}
