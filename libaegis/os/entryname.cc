//
//	aegis - project change supervisor
//	Copyright (C) 2002-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>

#include <libaegis/os.h>
#include <common/trace.h>


//
// NAME
//	os_entryname - take path apart
//
// SYNOPSIS
//	string_ty *os_entryname(string_ty *path);
//
// DESCRIPTION
//	Os_entryname is used to extract the entry part
//	from a pathname.
//
// RETURNS
//	pointer to dynamically allocated string.
//
// CAVEAT
//	Use str_free() when you are done with the return value.
//

string_ty *
os_entryname(string_ty *path)
{
    trace(("os_entryname(path = %08lX)\n{\n", (long)path));
    trace_string(path->str_text);
    string_ty *s = os_pathname(path, 1);
    const char *cp = strrchr(s->str_text, '/');
    if (cp && s->str_length > 1)
	path = str_from_c(cp + 1);
    else
	path = str_copy(s);
    str_free(s);
    trace_string(path->str_text);
    trace(("}\n"));
    return path;
}


nstring
os_entryname(const nstring &path)
{
    trace(("os_entryname(path = \"%s\")\n{\n", path.c_str()));
    nstring s = os_pathname(path, true);
    const char *cp = strrchr(s.c_str(), '/');
    if (cp && s.size() > 1)
	s = nstring(cp + 1);
    trace(("return \"%s\";\n", s.c_str()));
    trace(("}\n"));
    return s;
}
