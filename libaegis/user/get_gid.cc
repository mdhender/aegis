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
user_ty::get_gid()
    const
{
    trace(("user_ty::get_gid(this = %08lX)\n{\n", (long)this));
    trace(("return %d;\n", group_id));
    trace(("}\n"));
    return group_id;
}


int
user_gid(user_ty::pointer up)
{
    trace(("user_gid(up = %08lX)\n{\n", (long)up.get()));
    int result = up->get_gid();
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}
