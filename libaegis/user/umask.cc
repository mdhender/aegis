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
#include <libaegis/user.h>


int
user_ty::umask_get()
    const
{
    trace(("user_ty::umask_get(this = %08lX)\n{\n", (long)this));
    trace(("return %05o;\n", umask));
    trace(("}\n"));
    return umask;
}


void
user_ty::umask_set(int um)
{
    um |= 022;
    if (um & 1)
        um |= 4;
    umask = (um & 027);
}
