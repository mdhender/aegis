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

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/user.h>


nstring
user_ty::get_home()
    const
{
    trace(("user_ty::get_home(this = %08lX)\n{\n", (long)this));
    assert(!home.empty());
    trace(("return %s;\n", home.quote_c().c_str()));
    trace(("}\n"));
    return home;
}


string_ty *
user_home(user_ty::pointer up)
{
    static nstring result;
    trace(("user_home(up = %08lX)\n{\n", (long)up.get()));
    result = up->get_home();
    trace(("return %s;\n", result.quote_c().c_str()));
    trace(("}\n"));
    return result.get_ref();
}
