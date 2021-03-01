//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate becomes
//

#include <libaegis/change.h>
#include <common/error.h> // for assert
#include <libaegis/project.h>
#include <common/trace.h>
#include <libaegis/user.h>


void
change_become(change_ty *cp)
{
    trace(("change_become(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    project_become(cp->pp);
    trace(("}\n"));
}


void
change_become_undo()
{
    trace(("change_become_undo()\n{\n"));
    project_become_undo();
    trace(("}\n"));
}


void
change_developer_become(change_ty *cp)
{
    string_ty       *name;
    user_ty         *up;

    trace(("change_become(cp = %08lX)\n{\n", (long)cp));
    name = change_developer_name(cp);
    up = user_symbolic(cp->pp, name);
    user_become(up);
    trace(("}\n"));
}


void
change_developer_become_undo()
{
    trace(("change_become_undo()\n{\n"));
    user_become_undo();
    trace(("}\n"));
}
