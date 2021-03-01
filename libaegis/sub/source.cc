//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2008 Peter Miller
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
#include <common/str_list.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/cstate.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/sub/source.h>


//
// NAME
//	sub_source - the source substitution
//
// SYNOPSIS
//	string_ty *sub_source(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_source function implements the source substitution.
//	The source substitution is replaced by the path of the source file,
//	depending on wether it is in the baseline or the change.
//	If the file named in the argument is in the change,
//	the name will be left unchanged,
//	but if the file is in the baseline, an absolute path will resiult.
//	If the change is being integrated, it will always be left untouched.
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
sub_source(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_source()\n{\n"));

    //
    // Find the change.  If there is no change, it is also valid in
    // the baseline context.
    //
    bool absolute = false;
    change::pointer cp = sub_context_change_get(scp);
    if (!cp)
    {
	project_ty *pp = sub_context_project_get(scp);
	if (!pp)
	{
	    scp->error_set(i18n("not valid in current context"));
            trace(("return NULL;\n"));
            trace(("}\n"));
            return wstring();
	}
	cp = pp->change_get();
    }

    //
    // make sure we like the arguments.
    //
    switch (arg.size())
    {
    default:
	scp->error_set(i18n("requires one argument"));
        trace(("return NULL;\n"));
        trace(("}\n"));
        return wstring();

    case 2:
	break;

    case 3:
        {
            nstring s = arg[2].to_nstring();
            if (arglex_compare("Relative", s.c_str(), 0))
            {
                break;
            }
            if (arglex_compare("Absolute", s.c_str(), 0))
            {
                absolute = true;
                break;
            }
            scp->error_set
            (
                i18n("second argument must be \"Absolute\" or \"Relative\"")
            );
            trace(("return NULL;\n"));
            trace(("}\n"));
            return wstring();
        }
    }

    //
    // make sure we are in an appropriate state
    //
    cstate_ty *cstate_data = cp->cstate_get();
    if (cstate_data->state == cstate_state_awaiting_development)
    {
	scp->error_set(i18n("not valid in current context"));
        trace(("return NULL;\n"));
        trace(("}\n"));
        return wstring();
    }

    //
    // find the file's path
    //
    nstring fn = arg[1].to_nstring();
    nstring s(change_file_source(cp, fn.get_ref()));
    if (s.empty())
    {
	scp->error_set(i18n("source file unknown"));
        trace(("return NULL;\n"));
        trace(("}\n"));
        return wstring();
    }

    //
    // To turn absolute paths into relative ones, we need to see if
    // the file is in the first element of the search path.
    //
    if (!absolute)
    {
	string_list_ty search_path;
	if (cstate_data->state == cstate_state_completed)
	    project_search_path_get(cp->pp, &search_path, 0);
	else
	    change_search_path_get(cp, &search_path, 0);
	nstring s2(os_below_dir(nstring(search_path.string[0]), s));
	if (!s2.empty())
	{
	    s = fn;
	}
    }

    //
    // build the result
    //
    wstring result(s);
    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
