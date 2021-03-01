//
//	aegis - project change supervisor
//	Copyright (C) 2004 Walter Franzini;
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
// MANIFEST: implementation of the os_basename class
//

#include <ac/string.h>
#include <str.h>
#include <os.h>
#include <trace.h>


string_ty *
os_basename(string_ty *name, string_ty *ext)
{
    trace(("os_basename(name =\"%s\", ext = \"%s\")\n{\n", name->str_text,
           (ext ? ext->str_text : "(null)")));
    const char *cp = strrchr(name->str_text, '/');
    if (cp)
	++cp;
    else
	cp = name->str_text;

    const char *ep = name->str_text + name->str_length;
    if (NULL != ext && str_trailing_suffix(name, ext))
	ep -= ext->str_length;

    string_ty *path = str_n_from_c(cp, ep - cp);
    trace_string(path->str_text);
    trace(("}\n"));
    return path;
}
