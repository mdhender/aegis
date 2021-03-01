//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006 Peter Miller;
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
// MANIFEST: implementation of the sub_history_path class
//

#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/fstate.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/sub/history_path.h>


wstring_ty *
sub_history_path(sub_context_ty *scp, wstring_list_ty *arg)
{
    //
    // Find the change.  If there is no change, it is also valid in
    // the baseline context.
    //
    trace(("sub_history_path()\n{\n"));
    change_ty *cp = sub_context_change_get(scp);
    if (!cp)
    {
	project_ty *pp = sub_context_project_get(scp);
	if (!pp)
	{
	    sub_context_error_set(scp, i18n("not valid in current context"));
	    trace(("return NULL;\n"));
	    trace(("}\n"));
	    return 0;
	}
	cp = pp->change_get();
    }

    //
    // make sure we like the arguments.
    //
    if (arg->size() != 2)
    {
	sub_context_error_set(scp, i18n("requires one argument"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    //
    // make sure we are in an appropriate state
    //
    cstate_ty *cstate_data = change_cstate_get(cp);
    if (cstate_data->state == cstate_state_awaiting_development)
    {
	sub_context_error_set(scp, i18n("not valid in current context"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    //
    // find the file's latest source record
    //
    string_ty *fn = wstr_to_str(arg->get(1));
    fstate_src_ty *src = change_file_find(cp, fn, view_path_simple);
    if (!src)
    {
	str_free(fn);
	sub_context_error_set(scp, i18n("source file unknown"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    //
    // Use the source record to find the history path.
    //
    string_ty *hfn = project_history_filename_get(cp->pp, src);

    //
    // build the result
    //
    wstring_ty *result = str_to_wstr(hfn);
    str_free(fn);
    str_free(hfn);

    //
    // here for all exits
    //
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
