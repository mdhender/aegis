//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2008 Peter Miller
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
#include <common/error.h> // for assert
#include <libaegis/project.h>
#include <common/trace.h>
#include <libaegis/user.h>


void
change_become(change::pointer cp)
{
    trace(("change_become(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    project_become(cp->pp);
    trace(("}\n"));
}


void
change_become_undo(change::pointer cp)
{
    trace(("change_become_undo(cp)\n{\n"));
    project_become_undo(cp->pp);
    trace(("}\n"));
}


user_ty::pointer
change::developer_user_get()
{
    nstring name(change_developer_name(this));
    user_ty::pointer up = user_ty::create(name);
    up->set_gid(gid_get());
    up->umask_set(change_umask(this));
    return up;
}


void
change_developer_become(change::pointer cp)
{
    trace(("change_become(cp = %08lX)\n{\n", (long)cp));
    user_ty::pointer up = cp->developer_user_get();
    up->become_begin();
    trace(("}\n"));
}


void
change_developer_become_undo(change::pointer cp)
{
    trace(("change_become_undo(cp)\n{\n"));
    user_ty::pointer up = cp->developer_user_get();
    up->become_end();
    trace(("}\n"));
}
