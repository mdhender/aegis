/*
 *	aegis - project change supervisor
 *	Copyright (C) 1996, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate lefts
 */

#include <sub.h>
#include <sub/left.h>
#include <trace.h>
#include <wstr_list.h>


wstring_ty *
sub_left(sub_context_ty *scp, wstring_list_ty *arg)
{
	wstring_ty	*result;
	string_ty	*s;
	long		n;

	trace(("sub_left()\n{\n"/*}*/));
	if (arg->nitems != 3)
	{
		sub_context_error_set(scp, i18n("requires two arguments"));
		trace(("return NULL;\n"));
		trace((/*{*/"}\n"));
		return 0;
	}
	s = wstr_to_str(arg->item[2]);
	n = atol(s->str_text);
	trace(("n = %ld\n", n));
	str_free(s);
	if (n <= 0)
		result = wstr_from_c("");
	else if (n > (long)arg->item[1]->wstr_length)
		result = wstr_copy(arg->item[1]);
	else
		result = wstr_n_from_wc(arg->item[1]->wstr_text, (size_t)n);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}
