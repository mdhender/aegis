/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998 Peter Miller;
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
 * MANIFEST: functions to manipulate trim_extensis
 */

#include <mem.h>
#include <sub.h>
#include <sub/trim_extensi.h>
#include <trace.h>
#include <wstr_list.h>


wstring_ty *
sub_trim_extension(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wchar_t		*wcp;
	wstring_ty	*fn;
	wstring_ty	*result;

	trace(("sub_trim_extension()\n{\n"/*}*/));
	if (arg->nitems != 2)
	{
		sub_context_error_set(scp, i18n("requires one argument"));
		trace(("return NULL;\n"));
		trace((/*{*/"}\n"));
		return 0;
	}
	fn = arg->item[1];

	wcp = fn->wstr_text + fn->wstr_length;
	while (wcp > fn->wstr_text && wcp[-1] != '/')
	{
		--wcp;
		if (*wcp == '.')
		{
			if (wcp <= fn->wstr_text || wcp[-1] == '/')
				break;
			result =
				wstr_n_from_wc
				(
					fn->wstr_text,
					wcp - fn->wstr_text
				);
			trace(("return %8.8lX;\n", (long)result));
			trace((/*{*/"}\n"));
			return result;
		}
	}
	result = wstr_copy(fn);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}
