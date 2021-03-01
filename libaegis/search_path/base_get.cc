//
// aegis - project change supervisor
// Copyright (C) 2009 Walter Franzini
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <libaegis/os.h>
#include <libaegis/search_path/base_get.h>
#include <libaegis/user.h>


nstring
search_path_base_get(const nstring_list &search_path, user_ty::pointer up)
{
    //
    // Find the base for relative filenames.
    //
    bool based =
	(
	    search_path.size() >= 1
	&&
	    (
		up->relative_filename_preference
		(
		    uconf_relative_filename_preference_current
		)
	    ==
		uconf_relative_filename_preference_base
	    )
	);
    nstring base;
    if (based)
	base = search_path[0];
    else
    {
        user_ty::become scoped(up);

	base = nstring(os_curdir());
    }

    return base;
}
