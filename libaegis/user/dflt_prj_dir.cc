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
#include <libaegis/os.h>
#include <libaegis/user.h>


nstring
user_ty::default_project_directory()
{
    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    trace(("user_ty::default_project_directory(this = %08lX)\n{\n",
        (long)this));
    uconf_ty *ucp = uconf_get();
    nstring path(ucp->default_project_directory);
    if (path.empty())
	path = get_home();
    else if (path[0] != '/')
        path = os_path_join(get_home(), path);
    trace(("return %s;\n", path.quote_c().c_str()));
    trace(("}\n"));
    return path;
}
