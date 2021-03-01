/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002, 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate lock_prepares
 */

#include <change.h>
#include <error.h> /* for assert */
#include <lock.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


static void
waiting_callback(void *p)
{
    change_ty       *cp;

    cp = p;
    if (user_lock_wait(0))
	change_verbose(cp, 0, i18n("waiting for lock"));
    else
	change_verbose(cp, 0, i18n("lock not available"));
}


void
change_cstate_lock_prepare(change_ty *cp)
{
    trace(("change_cstate_lock_prepare(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    lock_prepare_cstate
    (
	project_name_get(cp->pp),
	cp->number,
	waiting_callback,
	(void *)cp
    );
    trace(("}\n"));
}
