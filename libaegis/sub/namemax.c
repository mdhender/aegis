/*
 *	aegis - project change supervisor
 *	Copyright (C) 1996 Peter Miller;
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
 * MANIFEST: functions to manipulate namemaxs
 */

#include <os.h>
#include <sub.h>
#include <sub/namemax.h>
#include <trace.h>
#include <wstr_list.h>


wstring_ty *
sub_namemax(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	string_ty	*s;
	int		n;
	wstring_ty	*result;

	trace(("sub_namemax()\n{\n"/*}*/));
	if (arg->nitems != 2)
	{
		sub_context_error_set(scp, i18n("requires one argument"));
		trace(("return NULL;\n"));
		trace((/*{*/"}\n"));
		return 0;
	}
	s = wstr_to_str(arg->item[1]);
	trace(("s = \"%s\";\n", s->str_text));
	os_become_orig();
	n = os_pathconf_name_max(s);
	os_become_undo();
	str_free(s);
	s = str_format("%d", n);
	trace(("result = \"%s\";\n", s->str_text));
	result = str_to_wstr(s);
	str_free(s);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}
