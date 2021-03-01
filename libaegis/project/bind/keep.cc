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
#include <libaegis/gonzo.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


void
project_ty::bind_keep(const nstring &home)
{
    trace(("project_ty::bind_keep(home = %s)\n{\n", home.quote_c().c_str()));
    if (gonzo_project_home_path_from_name(name_get()))
    {
        sub_context_ty sc;
        sc.var_set_string("Name", name_get());
        sc.fatal_intl(i18n("project $name exists"));
        // NOTREACHED
    }

    //
    // we need to set the user *before* we set the home directory,
    // because it goes into the directory and reads a few things.
    //
    up = user_ty::create();

    home_path_set(os_canonify_dirname(home));

    //
    // Now set the group from the directory's permissions
    //
    get_the_owner();
    if (uid != up->get_uid())
    {
        user_ty::pointer up1 = user_ty::create(uid);
        sub_context_ty sc;
        sc.var_set_string("FileName", home);
        sc.var_set_string("Number1", up1->name());
        sc.var_set_string("Number2", up->name());
        sc.fatal_intl(i18n("$filename: owner is $number1, should be $number2"));
        // NOTREACHED
    }
    up->set_gid(gid);
    up->umask_set(umask_get());
    trace(("}\n"));
}
