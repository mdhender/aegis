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
 * MANIFEST: functions to manipulate run_commands
 */

#include <change.h>
#include <change/env_set.h>
#include <error.h> /* for assert */
#include <os.h>
#include <sub.h>
#include <user.h>


static int run_test_command _((change_ty *, user_ty *, string_ty *,
	string_ty *, int, string_ty *));

static int
run_test_command(cp, up, filename, dir, inp, the_command)
	change_ty	*cp;
	user_ty		*up;
	string_ty	*filename;
	string_ty	*dir;
	int		inp;
	string_ty	*the_command;
{
	sub_context_ty	*scp;
	int		flags;
	int		result;

	assert(cp->reference_count >= 1);
	scp = sub_context_new();
	sub_var_set(scp, "File_Name", "%S", filename);
	the_command = substitute(scp, cp, the_command);
	sub_context_delete(scp);

	flags = inp ? OS_EXEC_FLAG_INPUT : OS_EXEC_FLAG_NO_INPUT;
	change_env_set(cp, 1);
	user_become(up);
	result = os_execute_retcode(the_command, flags, dir);
	os_become_undo();
	str_free(the_command);
	return result;
}


int
change_run_test_command(cp, up, filename, dir, inp)
	change_ty	*cp;
	user_ty		*up;
	string_ty	*filename;
	string_ty	*dir;
	int		inp;
{
	pconf		pconf_data;
	string_ty	*the_command;

	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 0);
	assert(pconf_data);
	the_command = pconf_data->test_command;
	assert(the_command);
	return run_test_command(cp, up, filename, dir, inp, the_command);
}


int
change_run_development_test_command(cp, up, filename, dir, inp)
	change_ty	*cp;
	user_ty		*up;
	string_ty	*filename;
	string_ty	*dir;
	int		inp;
{
	pconf		pconf_data;
	string_ty	*the_command;

	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 0);
	assert(pconf_data);
	the_command = pconf_data->development_test_command;
	assert(the_command);
	return run_test_command(cp, up, filename, dir, inp, the_command);
}
