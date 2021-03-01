//
// aegis - project change supervisor
// Copyright (C) 1999, 2004-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <libaegis/change.h>
#include <libaegis/os.h>
#include <libaegis/project.h>


int
change_pathconf_name_max(change::pointer cp)
{
    assert(cp->reference_count >= 1);
    string_ty *bl = cp->pp->baseline_path_get();
    string_ty *dd = change_development_directory_get(cp, 0);
    os_become_orig();
    int bl_max = os_pathconf_name_max(bl);
    int dd_max = os_pathconf_name_max(dd);
    os_become_undo();
    return (bl_max < dd_max ? bl_max : dd_max);
}


// vim: set ts=8 sw=4 et :
