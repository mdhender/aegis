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
 * MANIFEST: functions to manipulate reviewers
 */

#include <change.h>
#include <sub.h>
#include <sub/change/reviewer.h>
#include <sub/user.h>
#include <trace.h>
#include <user.h>
#include <wstr_list.h>


/*
 * NAME
 *	sub_reviewer - the reviewer substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_reviewer(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_reviewer function implements the reviewer substitution.
 *	The reviewer substitution is replaced by the name of the reviewer
 *	of the change.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_reviewer(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;
	string_ty	*s;
	change_ty	*cp;
	user_ty		*up;
	sub_user_func_ptr func;

	trace(("sub_reviewer()\n{\n"));
	cp = sub_context_change_get(scp);
	if (!cp)
	{
		yuck:
		sub_context_error_set
		(
			scp,
			i18n("not valid in current context")
		);
		result = 0;
	}
	else if (arg->nitems == 1)
	{
		s = change_reviewer_name(cp);
		if (!s)
			goto yuck;
		result = str_to_wstr(s);
		/* do not free s */
	}
	else if (arg->nitems == 2)
	{
		s = wstr_to_str(arg->item[1]);
		func = sub_user_func(s);
		str_free(s);
		if (!func)
		{
			sub_context_error_set
			(
				scp,
				i18n("unknown substitution variant")
			);
			result = 0;
		}
		else
		{
			up = user_symbolic(cp->pp, change_reviewer_name(cp));
			s = func(up);
			result = str_to_wstr(s);
			user_free(up);
			/* do not str_free(s) */
		}
	}
	else
	{
		sub_context_error_set
		(
			scp,
			i18n("requires one argument")
		);
		result = 0;
	}
	trace(("return %8.8lX;\n", (long)result));
	trace(("}\n"));
	return result;
}
