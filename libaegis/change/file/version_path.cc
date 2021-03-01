//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: functions to manipulate version_paths
//

#include <libaegis/change/file.h>
#include <libaegis/project/file.h>
#include <common/trace.h>


string_ty *
change_file_version_path(change_ty *cp, fstate_src_ty *src,  int *unlink_p)
{
    trace(("change_file_version_path(cp = %08lX, src = %08lX)\n{\n",
	(long)cp, (long)src));
    fstate_src_trace(src);
    if (!cp->bogus)
    {
	cstate_ty *cstate_data = change_cstate_get(cp);
	switch (cstate_data->state)
	{
	case cstate_state_awaiting_development:
	    break;

	case cstate_state_being_developed:
	case cstate_state_awaiting_review:
	case cstate_state_being_reviewed:
	case cstate_state_awaiting_integration:
	case cstate_state_being_integrated:
	    trace(("looking in change...\n"));
	    if (change_file_find(cp, src->file_name, view_path_first))
	    {
		// Note: it could be transparent.
		string_ty *rslt = change_file_path(cp, src->file_name);
		if (rslt)
		{
		    *unlink_p = 0;
		    trace(("return %08lX;\n", (long)rslt));
		    trace(("}\n"));
		    return rslt;
		}
	    }
	    break;

	case cstate_state_completed:
	    break;
	}
    }
    trace(("looking in project...\n"));
    string_ty *rslt = project_file_version_path(cp->pp, src, unlink_p);
    trace(("return %08lX;\n", (long)rslt));
    trace(("}\n"));
    return rslt;
}
