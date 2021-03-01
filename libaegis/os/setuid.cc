//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate setuids
//

#include <ac/errno.h>
#include <ac/stddef.h>
#include <ac/sys/types.h>
#include <ac/unistd.h>

#include <os.h>
#include <sub.h>


void
os_setuid(int uid)
{
    os_become_must_not_be_active();
    if (setuid(uid))
    {
	sub_context_ty  *scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_long(scp, "Argument", uid);
	fatal_intl(scp, i18n("setuid $arg: $errno"));
	// NOTREACHED
    }
}
