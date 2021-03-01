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
 * MANIFEST: functions to manipulate mkdir_betwens
 */

#include <error.h> /* for assert */
#include <os.h>
#include <trace.h>
#include <undo.h>


void
os_mkdir_between(top, extn, mode)
	string_ty	*top;
	string_ty	*extn;
	int		mode;
{
	char		*cp;
	string_ty	*s1;
	string_ty	*s2;

	trace(("os_mkdir_between(top = %08lX, extn = %08lX, \
mode = 0%o)\n{\n", top, extn, mode));
	trace_string(top->str_text);
	trace_string(extn->str_text);
	assert(top->str_text[0] == '/');
	assert(extn->str_text[0] != '/');
	assert(extn->str_text[extn->str_length - 1] != '/');
	assert(extn->str_length);
	for (cp = extn->str_text; *cp; ++cp)
	{
		if (*cp != '/')
			continue;
		s1 = str_n_from_c(extn->str_text, cp - extn->str_text);
		s2 = os_path_cat(top, s1);
		trace(("mkdir %s\n", s2->str_text));
		if (!os_exists(s2))
		{
			os_mkdir(s2, mode);
			undo_rmdir_errok(s2);
		}
		str_free(s1);
		str_free(s2);
	}
	trace(("}\n"));
}
