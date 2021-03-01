/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate identifiers
 */

#include <sub.h>
#include <sub/identifier.h>
#include <trace.h>
#include <wstr.h>
#include <wstr_list.h>


/*
 * NAME
 *	sub_identifier - the identifier substitution
 *
 * SYNOPSIS
 *	string_ty *sub_identifier(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_identifier function implements the identifier substitution.
 *	The identifier substitution is replaced by the single argument
 *	mapped to a C identifier.  All characters which are not legal in
 *	a C identifier are replaced by an underscore.
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
sub_identifier(scp, arg)
    sub_context_ty  *scp;
    wstring_list_ty *arg;
{
    wstring_ty	    *result;

    trace(("sub_identifier()\n{\n"));
    if (arg->nitems < 2)
    {
	sub_context_error_set(scp, i18n("requires one argument"));
	result = 0;
    }
    else
    {
	wstring_list_ty	results;
	size_t		j;

	wstring_list_constructor(&results);
	for (j = 1; j < arg->nitems; ++j)
	{
	    wstring_ty	*ws;

	    ws = wstr_to_ident(arg->item[j]);
	    wstring_list_append(&results, ws);
	    wstr_free(ws);
	}
	result = wstring_list_to_wstring(&results, 0, results.nitems, 0);
	wstring_list_destructor(&results);
    }
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
