//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2005 Peter Miller;
//	All rights reserved.
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
// MANIFEST: functions to manipulate projfileneeds
//

#include <libaegis/change.h>
#include <common/error.h>
#include <libaegis/project/history.h>


int
change_run_project_file_command_needed(change_ty *cp)
{
    cstate_ty       *cstate_data;

    assert(cp->reference_count >= 1);
    cstate_data = change_cstate_get(cp);
    return
	(
    	    cstate_data->project_file_command_sync
	!=
    	    project_last_change_integrated(cp->pp)
	);
}
