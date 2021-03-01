//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: implementation of the change_history_tran_beg_com class
//

#include <change.h>
#include <change/env_set.h>
#include <error.h> // for assert
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>


void
change_run_history_transaction_begin_command(change_ty *cp)
{
    trace(("change_run_history_transaction_begin_command(cp = %8.8lX)\n{\n",
	(long)cp));
    assert(cp->reference_count >= 1);
    pconf_ty *pconf_data = change_pconf_get(cp, 1);
    string_ty *the_command = pconf_data->history_transaction_begin_command;
    if (the_command && the_command->str_length)
    {
	the_command = substitute(0, cp, the_command);
	string_ty *dir = project_history_path_get(cp->pp);
	change_env_set(cp, 0);
	project_become(cp->pp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dir);
	project_become_undo();
	str_free(the_command);
    }
    trace(("}\n"));
}
