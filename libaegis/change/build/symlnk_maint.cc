//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/change/branch.h>
#include <libaegis/change.h>
#include <libaegis/pconf.h>
#include <libaegis/user.h>


void
change_maintain_symlinks_to_baseline(change::pointer cp, user_ty::pointer up,
    bool undoing)
{
    trace(("change_maintain_symlinks_to_baseline()\n{\n"));
    assert(cp->is_being_developed());
    assert(!change_was_a_branch(cp));
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    assert(pconf_data->development_directory_style);
    work_area_style_ty style = *pconf_data->development_directory_style;
    if (style.derived_at_start_only)
    {
	style.derived_file_link = false;
	style.derived_file_symlink = false;
	style.derived_file_copy = false;
    }
    if (undoing && (style.source_file_link || style.source_file_symlink))
    {
	style.derived_file_link = false;
	style.derived_file_symlink = false;
	style.derived_file_copy = false;
        style.source_file_link = false;
        style.source_file_symlink = false;
        style.source_file_copy = true;
    }
    if
    (
	style.source_file_link
    ||
	style.source_file_symlink
    ||
	style.source_file_copy
    ||
	style.derived_file_link
    ||
	style.derived_file_symlink
    ||
	style.derived_file_copy
    )
    {
	if (up->symlink_pref(!style.during_build_only))
	    change_create_symlinks_to_baseline(cp, up, style);
    }
    trace(("}\n"));
}
