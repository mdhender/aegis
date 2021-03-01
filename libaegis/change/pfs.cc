//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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

#include <libaegis/change/pfs.h>
#include <libaegis/project.h>
#include <libaegis/commit.h>


void
change_pfs_write(change::pointer cp)
{
    //
    // All it does is create a hard link.  This will be broken at the
    // next integrate pass, so it's quite cheap and quite quick, since
    // we have to write oput the project file state agin, anyway.
    //
    // Note that this means the delta's "psf" file is immutable, which
    // should give very good file-system cache behaviour.
    //
    // If we ever split the project file state into idividual filews
    // (because for most operations it will be faster) then this
    // function will need to be re-implemented.
    //
    pconf_ty *pconf_data = change_pconf_get(cp, 1);
    if (pconf_data->cache_project_file_list_for_each_delta)
    {
	change::pointer pcp = cp->pp->change_get();
	string_ty *fn1 = change_fstate_filename_get(pcp);
	string_ty *fn2 = change_pfstate_filename_get(cp);
	change_become(cp);

	//
        // Note: this will potentially have a bunch of "locked by" and
        // "about to be created by" settings, etc.
	// Will this be a problem?
	//
	commit_hard_link(fn1, fn2);

	change_become_undo(cp);
    }
}
