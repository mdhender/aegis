//
//	aegis - project change supervisor
//	Copyright (C) 1998, 2003, 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate trim_directos
//

#include <mem.h>
#include <sub.h>
#include <sub/trim_directo.h>
#include <trace.h>
#include <wstr/list.h>


wstring_ty *
sub_trim_directory(sub_context_ty *scp, wstring_list_ty *arg)
{
	wstring_ty	*dir;
	wchar_t		*wcp;
	wstring_ty	*result;
	string_ty	*s;
	long		n;

	trace(("sub_trim_directory()\n{\n"));
	n = 1;
	switch (arg->size())
	{
	default:
		sub_context_error_set(scp, i18n("requires one argument"));
		trace(("return NULL;\n"));
		trace(("}\n"));
		return 0;

	case 2:
		n = 1;
		dir = arg->get(1);
		break;

	case 3:
		s = wstr_to_str(arg->get(1));
		n = atol(s->str_text);
		trace(("n = %ld;\n", n));
		str_free(s);

		dir = arg->get(2);
		break;
	}

	//
	// Skip the given number of leading directory components.
	//
	wcp = dir->wstr_text;
	while (n-- > 0)
	{
		wchar_t *ep = wcp;
		while (*ep && *ep != '/')
			++ep;
		if (!*ep)
			break;
		wcp = ep + 1;
	}

	//
	// Build the result.
	//
	result = wstr_from_wc(wcp);
	trace(("return %8.8lX;\n", (long)result));
	trace(("}\n"));
	return result;
}
