//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
//	Copyright (C) 2007 Walter Franzini
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

#include <libaegis/http.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/invento_walk.h>


void
get_change_inventory(project_ty *pp, string_ty *, string_list_ty *modifiers)
{
    bool incl_orig = modifier_test(modifiers, "all");

    nstring project_name(pp->name_get());
    nstring cmd = nstring::format
        (
            "aedist --inventory --aeget --proj %s",
            project_name.c_str()
        );
    if (incl_orig)
        cmd += " --all";

    os_become_orig();
    nstring dot(os_curdir());
    os_execute(cmd, 0, dot);
    os_become_undo();
}
