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

#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/fstate.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/sub/history_path.h>


wstring
sub_history_path(sub_context_ty *scp, const wstring_list &arg)
{
    //
    // Find the change.  If there is no change, it is also valid in
    // the baseline context.
    //
    trace(("sub_history_path()\n{\n"));
    wstring result;
    change::pointer cp = sub_context_change_get(scp);
    if (!cp)
    {
	project_ty *pp = sub_context_project_get(scp);
	if (!pp)
	{
	    scp->error_set(i18n("not valid in current context"));
	    trace(("}\n"));
	    return result;
	}
	cp = pp->change_get();
    }

    //
    // make sure we like the arguments.
    //
    if (arg.size() != 2)
    {
	scp->error_set(i18n("requires one argument"));
	trace(("}\n"));
	return result;
    }

    //
    // make sure we are in an appropriate state
    //
    cstate_ty *cstate_data = cp->cstate_get();
    if (cstate_data->state == cstate_state_awaiting_development)
    {
	scp->error_set(i18n("not valid in current context"));
	trace(("}\n"));
	return result;
    }

    //
    // find the file's latest source record
    //
    nstring fn = arg[1].to_nstring();
    fstate_src_ty *src = change_file_find(cp, fn, view_path_simple);
    if (!src)
    {
	scp->error_set(i18n("source file unknown"));
	trace(("}\n"));
	return result;
    }

    //
    // Use the source record to find the history path.
    //
    nstring hfn(project_history_filename_get(cp->pp, src));

    //
    // build the result
    //
    result = wstring(hfn);

    //
    // here for all exits
    //
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
