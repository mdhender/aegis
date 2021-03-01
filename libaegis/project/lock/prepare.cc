//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller;
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
// MANIFEST: implementation of the project_lock_prepare class
//

#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/lock.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


static void
waiting_for_lock(void *p)
{
    project_ty *pp = (project_ty *)p;
    if (user_lock_wait(0))
	project_error(pp, 0, i18n("waiting for lock"));
    else
	project_fatal(pp, 0, i18n("lock not available"));
}


void
project_ty::pstate_lock_prepare()
{
    trace(("project_ty::pstate_lock_prepare(this = %08lX)\n{\n", (long)this));
    if (parent)
    {
	assert(pcp);
	change_cstate_lock_prepare(change_get());
    }
    else
	lock_prepare_pstate(name, waiting_for_lock, this);
    trace(("}\n"));
}
