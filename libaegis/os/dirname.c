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

#include <ac/string.h>

#include <error.h> /* for assert */
#include <os.h>
#include <trace.h>


/*
 * NAME
 *	os_dirname - take path apart
 *
 * SYNOPSIS
 *	string_ty *os_dirname(string_ty *path);
 *
 * DESCRIPTION
 *	Os_dirname is used to extract the directory part
 *	of a pathname.
 *
 * RETURNS
 *	pointer to dynamically allocated string.
 *
 * CAVEAT
 *	Use str_free() when you are done with the value returned.
 */

string_ty *
os_dirname(path)
	string_ty	*path;
{
	string_ty	*s;
	char		*cp;

	trace(("os_dirname(path = %08lX)\n{\n"/*}*/, path));
	trace_string(path->str_text);
	s = os_pathname(path, 1);
	cp = strrchr(s->str_text, '/');
	assert(cp);
	if (cp > s->str_text)
	{
		path = str_n_from_c(s->str_text, cp - s->str_text);
		str_free(s);
	}
	else
		path = s;
	trace_string(path->str_text);
	trace((/*{*/"}\n"));
	return path;
}
