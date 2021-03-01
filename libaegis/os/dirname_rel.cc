//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate dirnames
//

#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <common/trace.h>


//
// NAME
//	os_dirname_relative - take path apart
//
// SYNOPSIS
//	string_ty *os_dirname_relative(string_ty *path);
//
// DESCRIPTION
//	Os_dirname is used to extract the directory part
//	of a pathname.  If none, result is "."
//
// RETURNS
//	pointer to dynamically allocated string.
//
// CAVEAT
//	Use str_free() when you are done with the value returned.
//

string_ty *
os_dirname_relative(string_ty *path)
{
    trace(("os_dirname_relative(path = %08lX)\n{\n", (long)path));
    trace_string(path->str_text);
    const char *cp = strrchr(path->str_text, '/');
    string_ty *s = 0;
    if (!cp)
	    s = str_from_c(".");
    else if (cp == path->str_text)
	    s = str_from_c("/");
    else
	    s = str_n_from_c(path->str_text, cp - path->str_text);
    trace_string(s->str_text);
    trace(("}\n"));
    return s;
}


nstring
os_dirname_relative(const nstring &path)
{
    trace(("os_dirname_relative(path = \"%s\")\n{\n", path.c_str()));
    const char *cp = strrchr(path.c_str(), '/');
    if (!cp)
    {
	trace(("return \".\";\n}\n"));
	return ".";
    }
    if (cp == path.c_str())
    {
	trace(("return \"/\";\n}\n"));
	return "/";
    }
    nstring result(path.c_str(), cp - path.c_str());
    trace(("return \"%s\";\n", result.c_str()));
    trace(("}\n"));
    return result;
}
