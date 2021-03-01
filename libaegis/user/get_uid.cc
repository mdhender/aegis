//
//      aegis - project change supervisor
//      Copyright (C) 2007, 2008, 2012 Peter Miller
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

#include <common/trace.h>
#include <libaegis/user.h>


int
user_ty::get_uid()
    const
{
    trace(("user_ty::get_uid(this = %p)\n{\n", this));
    trace(("return %d;\n", user_id));
    trace(("}\n"));
    return user_id;
}


int
user_id(user_ty::pointer up)
{
    trace(("user_id(up = %p)\n{\n", up.get()));
    int result = up->get_uid();
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
