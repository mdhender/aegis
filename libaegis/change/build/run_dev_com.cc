//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate run_dev_coms
//

#include <change.h>
#include <change/env_set.h>
#include <error.h> // for assert
#include <os.h>
#include <str_list.h>
#include <sub.h>
#include <user.h>


void
change_run_development_build_command(change_ty *cp, user_ty *up,
    string_list_ty *partial)
{
    sub_context_ty  *scp;
    pconf_ty        *pconf_data;
    string_ty       *the_command;
    string_ty       *dd;

    assert(cp->reference_count >= 1);
    assert(change_is_being_developed(cp));
    pconf_data = change_pconf_get(cp, 1);
    assert(pconf_data);
    the_command = pconf_data->development_build_command;
    if (!the_command)
    {
	assert(pconf_data->errpos);
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", pconf_data->errpos);
	sub_var_set_charstar(scp, "FieLD_Name", "build_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	// NOTREACHED
	sub_context_delete(scp);
    }

    //
    // place the partial file name into the File_List variable
    //
    scp = sub_context_new();
    if (partial->nstrings)
    {
	string_ty	*s;

	s = wl2str(partial, 0, partial->nstrings, (char *)0);
	sub_var_set_string(scp, "File_List", s);
	str_free(s);
	sub_var_append_if_unused(scp, "File_List");
    }
    else
    {
	sub_var_set_charstar(scp, "File_List", "");
	sub_var_optional(scp, "File_List");
    }

    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);

    dd = change_development_directory_get(cp, 0);
    change_env_set(cp, 1);
    user_become(up);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dd);
    os_become_undo();
    str_free(the_command);
}