//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
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
#include <common/trace.h>
#include <libaegis/commit.h>
#include <libaegis/gonzo.h>
#include <libaegis/os.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>


void
user_ustate_write(user_ty::pointer up)
{
    up->ustate_write();
}


void
user_ty::ustate_write()
{
    trace(("user_ty::ustate_write(this = %08lX)\n{\n", (long)this));
    assert(ustate_data);
    assert(ustate_path);
    if (!ustate_modified)
    {
        trace(("}\n"));
        return;
    }

    //
    // write it out
    //
    static int count;
    nstring filename_new =
        nstring::format("%s,%d", ustate_path.c_str(), ++count);
    nstring filename_old =
        nstring::format("%s,%d", ustate_path.c_str(), ++count);
    gonzo_become();
    if (ustate_is_new)
    {
	undo_unlink_errok(filename_new);
	ustate_write_file(filename_new, ustate_data, 0);
	commit_rename(filename_new, ustate_path);
    }
    else
    {
	undo_unlink_errok(filename_new);
	ustate_write_file(filename_new, ustate_data, 0);
	commit_rename(ustate_path, filename_old);
	commit_rename(filename_new, ustate_path);
	commit_unlink_errok(filename_old);
    }
    os_chmod(filename_new, 0644);
    gonzo_become_undo();
    ustate_modified = false;
    ustate_is_new = false;
    trace(("}\n"));
}
