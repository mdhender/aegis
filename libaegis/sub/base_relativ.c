/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002, 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate base_relativs
 */

#include <arglex.h>
#include <change.h>
#include <change/file.h>
#include <cstate.h>
#include <os.h>
#include <project.h>
#include <project/file.h>
#include <sub.h>
#include <sub/base_relativ.h>
#include <str_list.h>
#include <trace.h>
#include <wstr_list.h>


/*
 * NAME
 *	sub_base_relative - the source substitution
 *
 * SYNOPSIS
 *	string_ty *sub_base_relative(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_base_relative function implements the base_relative
 *	substitution.  The base_relative substitution is replaced by the
 *	path of the source file, relative to the base of the project tree.
 *
 *	Requires exactly one argument.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_base_relative(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty	    *result;
    cstate_ty       *cstate_data;
    change_ty	    *cp;
    string_list_ty  search_path;
    string_ty	    *s;
    size_t	    j;
    size_t	    k;
    string_list_ty  results;

    /*
     * Find the change.	 If there is no change, it is also valid in
     * the baseline context.
     */
    trace(("sub_base_relative()\n{\n"));
    result = 0;
    cp = sub_context_change_get(scp);
    if (!cp)
    {
	project_ty	*pp;

	pp = sub_context_project_get(scp);
	if (!pp)
	{
	    sub_context_error_set(scp, i18n("not valid in current context"));
	    goto done;
	}
	cp = project_change_get(pp);
    }

    /*
     * make sure we like the arguments.
     */
    if (arg->nitems < 2)
    {
	sub_context_error_set(scp, i18n("requires one argument"));
	goto done;
    }

    /*
     * make sure we are in an appropriate state
     */
    cstate_data = change_cstate_get(cp);
    if (cstate_data->state == cstate_state_awaiting_development)
    {
	sub_context_error_set(scp, i18n("not valid in current context"));
	goto done;
    }

    /*
     * Get the search path.
     */
    if (cstate_data->state == cstate_state_completed)
    {
	string_list_constructor(&search_path);
	project_search_path_get(cp->pp, &search_path, 0);
    }
    else
	change_search_path_get(cp, &search_path, 0);

    /*
     * Turn the file name into an absolute path.
     */
    string_list_constructor(&results);
    for (k = 1; k < arg->nitems; ++k)
    {
	string_ty	    *fn;

	fn = wstr_to_str(arg->item[k]);
	change_become(cp);
	s = os_pathname(fn, 1);
	change_become_undo();
	str_free(fn);
	fn = s;

	/*
	 * Hunt down the search list, to see if the file is in any of those
	 * directories.
	 */
	for (j = 0; j < search_path.nstrings; ++j)
	{
	    s = os_below_dir(search_path.string[j], fn);
	    if (s)
	    {
		str_free(fn);
		if (s->str_length)
		    fn = s;
		else
		{
		    fn = str_from_c(".");
		    str_free(s);
		}
		break;
	    }
	}
	string_list_append(&results, fn);
	str_free(fn);
    }
    string_list_destructor(&search_path);

    /*
     * build the result
     */
    s = wl2str(&results, 0, results.nstrings, 0);
    result = str_to_wstr(s);
    str_free(s);

    /*
     * here for all exits
     */
    done:
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
