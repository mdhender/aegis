//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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

#include <common/arglex.h>
#include <common/nstring/list.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/cstate.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/sub/base_relativ.h>
#include <libaegis/sub.h>


//
// NAME
//	sub_base_relative - the source substitution
//
// SYNOPSIS
//	string_ty *sub_base_relative(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_base_relative function implements the base_relative
//	substitution.  The base_relative substitution is replaced by the
//	path of the source file, relative to the base of the project tree.
//
//	Requires exactly one argument.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring
sub_base_relative(sub_context_ty *scp, const wstring_list &arg)
{
    //
    // Find the change.	 If there is no change, it is also valid in
    // the baseline context.
    //
    trace(("sub_base_relative()\n{\n"));
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
    if (arg.size() < 2)
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
    // Get the search path.
    //
    string_list_ty search_path;
    if (cstate_data->state == cstate_state_completed)
	project_search_path_get(cp->pp, &search_path, 0);
    else
	change_search_path_get(cp, &search_path, 0);

    //
    // Turn the file name into an absolute path.
    //
    nstring_list results;
    for (size_t k = 1; k < arg.size(); ++k)
    {
	nstring fn = arg[k].to_nstring();
	change_become(cp);
	nstring s = os_pathname(fn, true);
	change_become_undo(cp);
	fn = s;

	//
	// Hunt down the search list, to see if the file is in any of those
	// directories.
	//
	for (size_t j = 0; j < search_path.nstrings; ++j)
	{
	    s = os_below_dir(nstring(search_path.string[j]), fn);
	    if (!s.empty())
	    {
                fn = s;
		break;
	    }
	}
	results.push_back(fn);
    }

    //
    // build the result
    //
    nstring s = results.unsplit();
    result = wstring(s);

    //
    // here for all exits
    //
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
