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
 * MANIFEST: functions to manipulate run_int_coms
 */

#include <change.h>
#include <change/env_set.h>
#include <error.h> /* for assert */
#include <os.h>
#include <pconf.h>
#include <project.h>
#include <sub.h>


void
change_run_build_command(cp)
	change_ty	*cp;
{
	sub_context_ty	*scp;
	pconf		pconf_data;
	string_ty	*the_command;
	string_ty	*id;

	assert(cp->reference_count >= 1);
	assert(cp->cstate_data);
	assert(cp->cstate_data->state == cstate_state_being_integrated);
	pconf_data = change_pconf_get(cp, 1);
	assert(pconf_data);
	the_command = pconf_data->build_command;
	assert(the_command);
	if (!the_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%s", THE_CONFIG_FILE);
		sub_var_set(scp, "FieLD_Name", "build_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * %1 = project name
	 * %2 = change number
	 * %3 = identifying string, in the form "a.b.Dnnn"
	 *	where 'a' is the major version number,
	 *	'b' is the minor version number,
	 *	and 'nnn' is the build number.
	 */
	scp = sub_context_new();
	sub_var_set(scp, "1", "${project}");
	sub_var_set(scp, "2", "${change}");
	sub_var_set(scp, "3", "${version}");
	sub_var_set(scp, "File_List", "");
	sub_var_optional(scp, "File_List");
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	id = change_integration_directory_get(cp, 0);
	change_env_set(cp, 1);
	project_become(cp->pp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, id);
	project_become_undo();
	str_free(the_command);
}
