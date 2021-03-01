//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2006, 2008 Peter Miller
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

#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <common/trace.h>


//
// NAME
//	os_dirname - take path apart
//
// SYNOPSIS
//	string_ty *os_dirname(string_ty *path);
//
// DESCRIPTION
//	Os_dirname is used to extract the directory part
//	of a pathname.
//
// RETURNS
//	pointer to dynamically allocated string.
//
// CAVEAT
//	Use str_free() when you are done with the value returned.
//

string_ty *
os_dirname(string_ty *path)
{
    trace(("os_dirname(path = %08lX)\n{\n", (long)path));
    trace_string(path->str_text);
    string_ty *s = os_pathname(path, 1);
    const char *cp = strrchr(s->str_text, '/');
    assert(cp);
    if (cp > s->str_text)
    {
	path = str_n_from_c(s->str_text, cp - s->str_text);
	str_free(s);
    }
    else
	path = s;
    trace_string(path->str_text);
    trace(("}\n"));
    return path;
}


nstring
os_dirname(const nstring &path)
{
    trace(("os_dirname(path = \"%s\")\n{\n", path.c_str()));
    nstring s = os_pathname(path, true);
    const char *cp = strrchr(s.c_str(), '/');
    assert(cp);
    if (cp && cp > s.c_str())
    {
	s = nstring(s.c_str(), cp - s.c_str());
    }
    trace(("return \"%s\";\n", s.c_str()));
    trace(("}\n"));
    return s;
}
