//
//	aegis - project change supervisor
//	Copyright (C) 2007 Peter Miller
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

#include <libaegis/os.h>
#include <libaegis/project/history.h>
#include <libaegis/user.h>


nstring
user_ty::default_development_directory(project_ty *pp)
{
    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    uconf_ty *ucp = uconf_get();
    nstring path(ucp->default_development_directory);
    if (path.empty())
    {
	path = nstring(project_default_development_directory_get(pp));
	if (path.empty())
	    path = get_home();
    }
    else
    {
	if (path[0] != '/')
	    path = os_path_join(get_home(), path);
    }
    return path;
}