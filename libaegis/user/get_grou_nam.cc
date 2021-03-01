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


nstring
user_ty::get_group_name()
    const
{
    trace(("user_ty::get_group_name(this = %08lX)\n{\n", (long)this));
    trace(("return %s;\n", group_name.quote_c().c_str()));
    trace(("}\n"));
    return group_name;
}


string_ty *
user_group(user_ty::pointer up)
{
    trace(("user_group(up = %08lX)\n{\n", (long)up.get()));
    nstring result = up->get_group_name();
    trace(("return %s;\n", result.quote_c().c_str()));
    trace(("}\n"));
    return result.get_ref();
}
