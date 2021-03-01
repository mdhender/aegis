//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate search_paths
//

#include <change.h>
#include <change/file.h>
#include <project.h>
#include <project/file.h>
#include <str_list.h>
#include <sub.h>
#include <sub/search_path.h>
#include <trace.h>
#include <wstr_list.h>


//
// NAME
//	sub_search_path - the search_path substitution
//
// SYNOPSIS
//	string_ty *sub_search_path(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_search_path function implements the search_path
//	substitution.  The search_path substitution is replaced by a
//	colon separated list of absolute paths to search when building a
//	change, it will point from a change to its branch and so on up
//	to the project trunk.
//
//	Requires exactly zero arguments.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_search_path(sub_context_ty *scp, wstring_list_ty *arg)
{
	wstring_ty	*result;
	change_ty	*cp;
	string_list_ty	tmp;
	string_ty	*s;
	project_ty	*pp;

	trace(("sub_search_path()\n{\n"));
	result = 0;
	if (arg->nitems != 1)
	{
		sub_context_error_set(scp, i18n("requires zero arguments"));
		goto done;
	}

	cp = sub_context_change_get(scp);
	if (!cp)
	{
		pp = sub_context_project_get(scp);
		if (!pp)
		{
			sub_context_error_set
			(
				scp,
				i18n("not valid in current context")
			);
			goto done;
		}

		string_list_constructor(&tmp);
		project_search_path_get(pp, &tmp, 0);
	}
	else
		change_search_path_get(cp, &tmp, 0);

	s = wl2str(&tmp, 0, tmp.nstrings, ":");
	string_list_destructor(&tmp);
	result = str_to_wstr(s);
	str_free(s);
done:
	trace(("return %8.8lX;\n", (long)result));
	trace(("}\n"));
	return result;
}
