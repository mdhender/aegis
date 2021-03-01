/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998, 2003 Peter Miller;
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
 * MANIFEST: functions to implement the quote substitution
 */

#include <sub.h>
#include <sub/quote.h>
#include <trace.h>
#include <wstr_list.h>


wstring_ty *
sub_quote(sub_context_ty *scp, wstring_list_ty *arg)
{
	wstring_ty	*s;
	wstring_ty	*result;

	/*
	 * Convert the work list to a single string.
	 */
	trace(("sub_quote()\n{\n"/*}*/));
	s = wstring_list_to_wstring(arg, (size_t)1, arg->nitems, (char *)0);

	/*
	 * quote the string as required
	 */
	result = wstr_quote_shell(s);

	/*
	 * clean up and return
	 */
	wstr_free(s);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}
