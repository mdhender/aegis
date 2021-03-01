//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006 Peter Miller
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
// MANIFEST: implementation of the change_identifi_sub_get_historia class
//

#include <libaegis/change/branch.h>
#include <libaegis/change/identifi_sub.h>
#include <libaegis/project/file/roll_forward.h>
#include <common/trace.h>


project_file_roll_forward *
change_identifier_subset::get_historian()
{
    //
    // Need to reconstruct the appropriate file histories.
    //
    trace(("project = \"%s\"\n", project_name_get(pid.get_pp())->str_text));
    if (!historian_p)
    {
	historian_p =
	    new project_file_roll_forward
	    (
		pid.get_pp(),
		(
		    delta_date != NO_TIME_SET
		?
		    delta_date
		:
		    change_completion_timestamp(get_cp())
		),
		0
	    );
	trace_pointer(historian_p);
    }
    return historian_p;
}
