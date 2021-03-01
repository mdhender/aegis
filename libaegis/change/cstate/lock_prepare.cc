//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2002-2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/lock.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


static void
waiting_callback(void *p)
{
    change::pointer cp;

    cp = (change::pointer )p;
    if (user_ty::create()->lock_wait())
        change_verbose(cp, 0, i18n("waiting for lock"));
    else
        change_verbose(cp, 0, i18n("lock not available"));
}


void
change_cstate_lock_prepare(change::pointer cp)
{
    trace(("change_cstate_lock_prepare(cp = %p)\n{\n", cp));
    assert(cp->reference_count >= 1);
    lock_prepare_cstate
    (
        project_name_get(cp->pp).get_ref(),
        cp->number,
        waiting_callback,
        (void *)cp
    );
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
