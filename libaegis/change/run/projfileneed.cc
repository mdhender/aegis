//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2008 Peter Miller
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

#include <common/error.h>
#include <libaegis/change.h>
#include <libaegis/lock.h>
#include <libaegis/project/history.h>


bool
change::run_project_file_command_needed()
{
    assert(lock_active());
    assert(reference_count >= 1);
    cstate_ty *csp = cstate_get();
    return
	(
    	    csp->project_file_command_sync
	!=
    	    project_last_change_integrated(pp)
	);
}


void
change::run_project_file_command_done()
{
    assert(reference_count >= 1);
    cstate_ty *csp = cstate_get();
    csp->project_file_command_sync = project_last_change_integrated(pp);
}
