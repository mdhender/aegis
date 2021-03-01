//
//	aegis - project change supervisor
//	Copyright (C) 2004 Walter Franzini
//	Copyright (C) 2008 Peter Miller
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
#include <common/str.h>
#include <libaegis/os.h>
#include <common/trace.h>


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


nstring
os_basename(const nstring &name, const nstring &ext)
{
    trace(("os_basename(name =\"%s\", ext = \"%s\")\n{\n",
	name.c_str(), ext.c_str()));
    const char *cp = strrchr(name.c_str(), '/');
    if (cp)
	++cp;
    else
	cp = name.c_str();

    const char *ep = name.c_str() + name.size();
    if (ep >= cp + ext.size() && name.ends_with(ext))
    {
	ep -= ext.size();
    }

    nstring result(cp, ep - cp);
    trace(("return \"%s\";\n", result.c_str()));
    trace(("}\n"));
    return result;
}
