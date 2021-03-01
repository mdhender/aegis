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
 * MANIFEST: functions to manipulate entrynam_rels
 */

#include <ac/string.h>

#include <os.h>
#include <trace.h>


string_ty *
os_entryname_relative(path)
	string_ty	*path;
{
	char		*cp;

	trace(("os_entryname_relative(path = %08lX)\n{\n", (long)path));
	trace_string(path->str_text);
	cp = strrchr(path->str_text, '/');
	if (cp)
		path = str_from_c(cp + 1);
	else
		path = str_copy(path);
	trace_string(path->str_text);
	trace(("}\n"));
	return path;
}
