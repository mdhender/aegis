/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate capitalizes
 */

#include <mem.h>
#include <sub.h>
#include <sub/capitalize.h>
#include <trace.h>
#include <wstr_list.h>


/*
 * NAME
 *	sub_capitalize - the capitalize substitution
 *
 * SYNOPSIS
 *	string_ty *sub_capitalize(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_capitalize function implements the capitalize substitution.
 *	The capitalize substitution is replaced by the single argument
 *	mapped to upper case.
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
sub_capitalize(scp, arg)
    sub_context_ty  *scp;
    wstring_list_ty *arg;
{
    wstring_ty	    *result;

    trace(("sub_capitalize()\n{\n"/*}*/));
    if (arg->nitems < 2)
    {
	sub_context_error_set(scp, i18n("requires one argument"));
	result = 0;
    }
    else
    {
	wstring_ty	*ws;

	ws = wstring_list_to_wstring(arg, 1, arg->nitems, 0);
	result = wstr_capitalize(ws);
	wstr_free(ws);
    }
    trace(("return %8.8lX;\n", (long)result));
    trace((/*{*/"}\n"));
    return result;
}
