//
//	aegis - project change supervisor
//	Copyright (C) 2003-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/change/env_set.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


void
change::run_make_transparent_undo_command(string_list_ty *wlp,
    const user_ty::pointer &up)
{
    trace(("change::run_make_transparent_undo_command(this = %08lX)\n{\n",
	(long)this));

    assert(!lock_active());
    assert(reference_count >= 1);
    pconf_ty *pcp = change_pconf_get(this, 0);
    string_ty *the_command = pcp->make_transparent_undo_command;
    if (!the_command)
	the_command = pcp->change_file_undo_command;
    if (!the_command)
	the_command = pcp->change_file_command;
    if (!the_command || !the_command->str_length)
    {
        trace(("}\n"));
        return;
    }

    sub_context_ty sc;
    string_ty *the_files = wlp->unsplit();
    sc.var_set_string("File_List", the_files);
    sc.var_optional("File_List");
    str_free(the_files);
    the_command = sc.substitute(this, the_command);

    string_ty *dd = change_development_directory_get(this, 0);
    change_env_set(this, 0);
    user_ty::become scoped(up);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT | OS_EXEC_FLAG_ERROK, dd);
    str_free(the_command);
    trace(("}\n"));
}
