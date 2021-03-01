//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
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

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


pstate_ty *
project_ty::pstate_get()
{
    trace(("project_ty::pstate_get(this = %08lX)\n{\n", (long)this));
    if (parent)
	this_is_a_bug();
    lock_sync();
    if (!pstate_data)
    {
	string_ty	*path;

	path = pstate_path_get();
	is_a_new_file = 0;

	//
	// can't become the project, because don't know who
	// the project is, yet.
	//
	// This also means we can use UNIX system security
	// to exclude unwelcome access.
	//
	get_the_owner();
	os_become_orig();
	os_chown_check(path, 0, uid, gid);
	pstate_data = pstate_read_file(path);
	os_become_undo();

	if (!pstate_data->next_test_number)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", pstate_path_get());
	    sub_var_set_charstar(scp, "FieLD_Name", "next_test_number");
	    project_fatal
	    (
		this,
		scp,
		i18n("$filename: corrupted \"$field_name\" field")
	    );
	    // NOTREACHED
	    sub_context_delete(scp);
	}

	if (pstate_data->next_change_number)
	    convert_to_new_format();
    }
    trace(("return %08lX;\n", (long)pstate_data));
    trace(("}\n"));
    return pstate_data;
}
