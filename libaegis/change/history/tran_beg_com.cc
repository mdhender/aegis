//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/change.h>
#include <libaegis/change/env_set.h>
#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
change_run_history_transaction_begin_command(change::pointer cp)
{
    trace(("change_run_history_transaction_begin_command(cp = %8.8lX)\n{\n",
	(long)cp));
    assert(cp->reference_count >= 1);
    pconf_ty *pconf_data = change_pconf_get(cp, 1);
    string_ty *the_command = pconf_data->history_transaction_begin_command;
    if (the_command && the_command->str_length)
    {
	the_command = substitute(0, cp, the_command);
	string_ty *dir = cp->pp->history_path_get();
	change_env_set(cp, 0);
	project_become(cp->pp);
	os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dir);
	project_become_undo(cp->pp);
	str_free(the_command);
    }
    trace(("}\n"));
}
