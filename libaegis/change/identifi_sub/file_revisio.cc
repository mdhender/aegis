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
#include <libaegis/change/file.h>
#include <libaegis/change/functor.h>
#include <libaegis/change.h>
#include <libaegis/change/identifi_sub.h>
#include <libaegis/cstate.h>
#include <libaegis/file/event.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project/identifi_sub.h>
#include <libaegis/sub.h>


file_revision
change_identifier_subset::get_file_revision(const nstring &filename,
    change_functor &bad_state)
{
    //
    // Check the change state.
    //
    trace(("change_identifier_subset::get_file_revision(\"%s\")\n{\n",
	filename.c_str()));
    get_cp();
    assert(cp);
    cstate_ty *cstate_data = cp->cstate_get();
    fstate_src_ty *src = 0;
    switch (cstate_data->state)
    {
#ifndef DEBUG
    default:
#endif
    case cstate_state_awaiting_development:
	bad_state(cp);
	trace(("}\n"));
	return file_revision("/dev/null", false);

    case cstate_state_completed:
	{
	    //
	    // Need to reconstruct the appropriate file histories.
	    //
	    trace(("project = \"%s\"\n",
		project_name_get(pid.get_pp())->str_text));
	    project_file_roll_forward *hp = get_historian();
	    file_event *fep = hp->get_last(filename.get_ref());
	    if (!fep)
	    {
		//
		// The file doesn't exist yet at this
		// delta.  Omit it.
		//
	       	trace(("}\n"));
		return file_revision("/dev/null", false);
	    }
	    src = fep->get_src();
	}
	break;

    case cstate_state_being_integrated:
    case cstate_state_awaiting_integration:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_review:
    case cstate_state_being_developed:
	trace(("mark\n"));
	src = change_file_find(cp, filename.get_ref(), view_path_simple);
	if (!src)
	{
	    sub_context_ty *scp = sub_context_new();
	    src = change_file_find_fuzzy(cp, filename.get_ref());
	    if (!src)
	    {
		src =
		    pid.get_pp()->file_find_fuzzy
		    (
			filename.get_ref(),
			view_path_extreme
		    );
	    }
	    sub_var_set_string(scp, "File_Name", filename.get_ref());
	    if (src)
	    {
		sub_var_set_string(scp, "Guess", src->file_name);
		change_fatal(cp, scp, i18n("no $filename, closest is $guess"));
	    }
	    else
	    {
		change_fatal(cp, scp, i18n("no $filename"));
	    }
	}
	break;
    }
    trace(("src = %08lX\n", (long)src));
    assert(src);
    switch (src->action)
    {
    case file_action_remove:
	//
	// The file had been removed at this
	// delta.  Omit it.
	//
	trace(("}\n"));
	return file_revision("/dev/null", false);

    case file_action_create:
    case file_action_modify:
    case file_action_insulate:
    case file_action_transparent:
	break;
    }
    int from_unlink = 0;
    trace(("edit = %s\n", src->edit->revision->str_text));
    nstring from(change_file_version_path(cp, src, &from_unlink));
    trace(("}\n"));
    return file_revision(from, from_unlink);
}
