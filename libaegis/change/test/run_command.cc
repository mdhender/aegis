//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2000, 2003, 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate run_commands
//

#include <ac/stdio.h>

#include <change.h>
#include <change/env_set.h>
#include <error.h> // for assert
#include <os.h>
#include <project/file.h>
#include <str_list.h>
#include <sub.h>
#include <user.h>


static int
run_test_command(change_ty *cp, user_ty *up, string_ty *filename,
    string_ty *dir, int inp, string_ty *the_command, int bl)
{
    sub_context_ty  *scp;
    int             flags;
    int             result;

    assert(cp->reference_count >= 1);
    scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", filename);

    if (bl && !cp->bogus)
    {
	string_list_ty spbl;
	project_search_path_get(cp->pp, &spbl, 0);
	string_ty *s = spbl.unsplit(":");
	sub_var_set_string(scp, "Search_Path_Executable", s);
	str_free(s);
	sub_var_override(scp, "Search_Path_Executable");
	sub_var_optional(scp, "Search_Path_Executable");
    }
    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);
    scp = 0;

    flags = inp ? OS_EXEC_FLAG_INPUT : OS_EXEC_FLAG_NO_INPUT;
    change_env_set(cp, 1);
    user_become(up);
    result = os_execute_retcode(the_command, flags, dir);
    os_become_undo();
    str_free(the_command);
    return result;
}


int
change_run_test_command(change_ty *cp, user_ty *up, string_ty *filename,
    string_ty *dir, int inp, int bl)
{
    pconf_ty        *pconf_data;
    string_ty       *the_command;

    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 0);
    assert(pconf_data);
    the_command = pconf_data->test_command;
    assert(the_command);
    return run_test_command(cp, up, filename, dir, inp, the_command, bl);
}


int
change_run_development_test_command(change_ty *cp, user_ty *up,
    string_ty *filename, string_ty *dir, int inp, int bl)
{
    pconf_ty        *pconf_data;
    string_ty       *the_command;

    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 0);
    assert(pconf_data);
    the_command = pconf_data->development_test_command;
    assert(the_command);
    return run_test_command(cp, up, filename, dir, inp, the_command, bl);
}
