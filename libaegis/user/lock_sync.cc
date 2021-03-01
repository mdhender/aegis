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

#include <common/trace.h>
#include <libaegis/lock.h>
#include <libaegis/user.h>


void
user_ty::lock_sync()
{
    trace(("user_ty::lock_sync(this = %08lX)\n{\n", (long)this));
    long n = ::lock_magic();
    if (lock_magic != n)
    {
	lock_magic = n;
	if (ustate_data && !ustate_is_new)
	{
	    ustate_type.free(ustate_data);
	    ustate_data = 0;
	}
    }
    trace(("}\n"));
}
