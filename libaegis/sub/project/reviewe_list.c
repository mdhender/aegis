/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate reviewe_lists
 */

#include <project_hist.h>
#include <str_list.h>
#include <sub.h>
#include <sub/project/reviewe_list.h>
#include <trace.h>
#include <wstr.h>
#include <wstr_list.h>


/*
 * NAME
 *	sub_reviewer_list - the reviewer_list substitution
 *
 * SYNOPSIS
 *	string_ty *sub_reviewer_list(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_reviewer_list function implements the reviewer_list
 *	substitution.  The reviewer_list substitution is replaced by a
 *	space separated list of the project's reviewers.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_reviewer_list(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	long		j;
	string_list_ty	wl;
	string_ty	*s;
	project_ty	*pp;

	trace(("sub_reviewer_list()\n{\n"));
	if (arg->nitems != 1)
	{
		sub_context_error_set(scp, i18n("requires zero arguments"));
		result = 0;
		goto done;
	}
	pp = sub_context_project_get(scp);
	if (!pp)
	{
		sub_context_error_set(scp, i18n("not valid in current context"));
		result = 0;
		goto done;
	}

	/*
	 * build a string containing all of the project reviewers
	 */
	string_list_constructor(&wl);
	for (j = 0; ; ++j)
	{
		s = project_reviewer_nth(pp, j);
		if (!s)
			break;
		string_list_append(&wl, s);
	}
	s = wl2str(&wl, 0, wl.nstrings, " ");
	string_list_destructor(&wl);
	result = str_to_wstr(s);
	str_free(s);

	done:
	trace(("return %8.8lX;\n", (long)result));
	trace(("}\n"));
	return result;
}
