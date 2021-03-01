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
 * MANIFEST: functions to manipulate dirnames
 */

#include <os.h>
#include <str.h>
#include <sub.h>
#include <sub/dirname.h>
#include <trace.h>
#include <wstr_list.h>


/*
 * NAME
 *	sub_dirname - the dirname substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_dirname(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_dirname function implements the dirname substitution.
 *	The dirname substitution is replaced by the dirname of
 *	the argument path, similar to the dirname(1) command.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_dirname(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_dirname()\n{\n"/*}*/));
	if (arg->nitems != 2)
	{
		sub_context_error_set(scp, i18n("requires one argument"));
		result = 0;
	}
	else
	{
		string_ty	*s1;
		string_ty	*s2;

		s1 = wstr_to_str(arg->item[1]);
		os_become_orig();
		s2 = os_dirname(s1);
		os_become_undo();
		str_free(s1);
		result = str_to_wstr(s2);
		str_free(s2);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}
