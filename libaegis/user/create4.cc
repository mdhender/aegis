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
#include <libaegis/os.h>
#include <libaegis/user.h>


user_ty::pointer
user_executing()
{
    trace(("user_ty::create()\n{\n"));
    user_ty::pointer result = user_ty::create();
    trace(("return %p;\n", result.get()));
    trace(("}\n"));
    return result;
}


user_ty::pointer
user_ty::create()
{
    trace(("user_ty::create()\n{\n"));
    int uid = 0;
    int gid = 0;
    os_become_orig_query(&uid, &gid, (int *)0);
    user_ty::pointer result = user_ty::create(uid);
    trace(("return %p;\n", result.get()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
