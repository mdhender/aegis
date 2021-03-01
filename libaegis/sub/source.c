/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate sources
 */

#include <arglex.h>
#include <change.h>
#include <change/file.h>
#include <cstate.h>
#include <os.h>
#include <project.h>
#include <project/file.h>
#include <sub.h>
#include <sub/source.h>
#include <str_list.h>
#include <trace.h>
#include <wstr_list.h>


/*
 * NAME
 *	sub_source - the source substitution
 *
 * SYNOPSIS
 *	string_ty *sub_source(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_source function implements the source substitution.
 *	The source substitution is replaced by the path of the source file,
 *	depending on wether it is in the baseline or the change.
 *	If the file named in the argument is in the change,
 *	the name will be left unchanged,
 *	but if the file is in the baseline, an absolute path will resiult.
 *	If the change is being integrated, it will always be left untouched.
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
sub_source(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	cstate		cstate_data;
	int		absolute;
	string_ty	*fn;
	string_ty	*s;
	change_ty	*cp;

	/*
	 * Find the change.  If there is no change, it is also valid in
	 * the baseline context.
	 */
	trace(("sub_source()\n{\n"));
	absolute = 0;
	result = 0;
	cp = sub_context_change_get(scp);
	if (!cp)
	{
		project_ty	*pp;

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
		cp = project_change_get(pp);
	}

	/*
	 * make sure we like the arguments.
	 */
	switch (arg->nitems)
	{
	default:
		sub_context_error_set(scp, i18n("requires one argument"));
		goto done;

	case 2:
		break;

	case 3:
		s = wstr_to_str(arg->item[2]);
		if (arglex_compare("Relative", s->str_text))
		{
			str_free(s);
			break;
		}
		if (arglex_compare("Absolute", s->str_text))
		{
			str_free(s);
			absolute = 1;
			break;
		}
		str_free(s);
		sub_context_error_set
		(
			scp,
		    i18n("second argument must be \"Absolute\" or \"Relative\"")
		);
		goto done;
	}

	/*
	 * make sure we are in an appropriate state
	 */
	cstate_data = change_cstate_get(cp);
	if (cstate_data->state == cstate_state_awaiting_development)
	{
		sub_context_error_set
		(
			scp,
			i18n("not valid in current context")
		);
		goto done;
	}

	/*
	 * find the file's path
	 */
	fn = wstr_to_str(arg->item[1]);
	s = change_file_source(cp, fn);
	if (!s)
	{
		str_free(fn);
		sub_context_error_set(scp, i18n("source file unknown"));
		goto done;
	}

	/*
	 * To turn absolute paths into relative ones, we need to see if
	 * the file is in the first element of the search path.
	 */
	if (!absolute)
	{
		string_list_ty	search_path;
		string_ty	*s2;

		if (cstate_data->state == cstate_state_completed)
		{
			string_list_constructor(&search_path);
			project_search_path_get(cp->pp, &search_path, 0);
		}
		else
			change_search_path_get(cp, &search_path, 0);
		s2 = os_below_dir(search_path.string[0], s);
		if (s2)
		{
			str_free(s2);
			str_free(s);
			s = str_copy(fn);
		}
		string_list_destructor(&search_path);
	}

	/*
	 * build the result
	 */
	result = str_to_wstr(s);
	str_free(fn);
	str_free(s);

	/*
	 * here for all exits
	 */
	done:
	trace(("return %8.8lX;\n", (long)result));
	trace(("}\n"));
	return result;
}
